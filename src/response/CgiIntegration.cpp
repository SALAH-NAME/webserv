#include "ResponseHandler.hpp"

Pipe &ResponseHandler::GetCgiInPipe(){return (CgiObj.GetInPipe());}

Pipe &ResponseHandler::GetCgiOutPipe(){return (CgiObj.GetOutPipe());}

pid_t ResponseHandler::GetCgiChildPid(){return (CgiObj.GetChildPid());}

bool ResponseHandler::ReachedCgiBodyPhase(){return CgiObj.ReachedBodyPhase();}


bool ResponseHandler::RequireCgi()
{
	return (CgiObj.GetExecutionStartTime() != -1);
}

bool ResponseHandler::IsCgiChildRunning()
{
	UpdateCgiChildExitStatus();

	return (child_status == -1);
}

bool ResponseHandler::CheckCgiTimeOut()
{
	if (!IsCgiChildRunning())
		return false;
	return (std::time(NULL) - CgiObj.GetExecutionStartTime()
                >= loc_config->getCgiTimeout());
}

void	ResponseHandler::UpdateCgiChildExitStatus()//returns -1 if cgi child still running 
{
	int		exit_status;
	pid_t	child_pid = GetCgiChildPid();
	int		wait_rval;

	if (child_status != -1)//child has been already reaped
		return ;
	wait_rval = waitpid(child_pid, &exit_status, WNOHANG);
	if (wait_rval == 0)// child still running
		child_status = -1;//keep -1 to signify that the child is not reaped yet
	else if (wait_rval == child_pid) // child exited
		child_status = WEXITSTATUS(exit_status);
	else
		throw (ResponseHandlerError(req->getVersion() + " 502 Bad Gateway", 502));
}

void ResponseHandler::CheckCgiChildState() // use only if cgi is required
{
	UpdateCgiChildExitStatus();
	if (!IsCgiChildRunning() && child_status != 0){
		DeleteCgiTargetFile();
		// std::cout << "script failed" << std::endl;//logger
		throw (ResponseHandlerError(req->getVersion() + " 502 Bad Gateway", 502));
	}
	else if (CheckCgiTimeOut()){
		// std::cout << "time out" << std::endl;//logger
		DeleteCgiTargetFile(); 
		CgiObj.KillChild();
		throw (ResponseHandlerError(req->getVersion() + " 504 Gateway Timeout", 504));
	}
}

std::string	ResponseHandler::GenerateCgiStatusLine()
{
	int 		status_code = CgiObj.GetStatusCode();
	std::string	reason_phrase = CgiObj.GetReasonPhrase();

	if (status_code == 0)
		return (req->getVersion() + " 200 OK");
	if (!reason_phrase.empty())
		return (req->getVersion() + " " + NumtoString(status_code) + reason_phrase);
	else if (status_phrases.find(status_code) != status_phrases.end())
		return (req->getVersion() + " " + NumtoString(status_code) + ' ' + status_phrases[status_code]);
	else
		return req->getVersion() + " " + NumtoString(status_code) + ' ' + "Unknown";
}

void ResponseHandler::GenerateHeaderFromCgiData()
{
	std::map<std::string, std::string> &headers = CgiObj.GetOutputHeaders();
	std::vector<std::string> &extra_cookies = CgiObj.GetExtraCookieValues();
	bool has_date = headers.find("date") != headers.end();
	bool has_name = headers.find("server") != headers.end();
	bool has_connection = headers.find("connection") != headers.end();

	response_header += GenerateCgiStatusLine() + CRLF;
	response_header += has_date ? "" : "Date: " + GenerateTimeStamp() + CRLF;
	response_header += has_name ? "" : "Server: " + std::string(SRV_NAME) + CRLF;
	response_header += has_connection ? "" : (keep_alive ? "Connection: keep_alive\r\n" : "Connection: close\r\n");
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		response_header += it->first + ": " + it->second + CRLF;
	for (std::vector<std::string>::iterator it = extra_cookies.begin(); it != extra_cookies.end(); it++)
		response_header += "Set-Cookie: " + *it + CRLF;
}

void ResponseHandler::FinishCgiResponse()//if an exception is thrown call loadErrorPage
{
	int returned_length = CgiObj.GetContentLength();
	if (returned_length == -1)
		response_header += "Content-Length: " + NumtoString(cgi_buffer_size + response_body.size())+CRLF+CRLF;
	else if (cgi_buffer_size + response_body.size() != (unsigned)returned_length){
		DeleteCgiTargetFile();
		CgiObj.KillChild();
		// std::cout << "cgi returned content-lenght not true" << std::endl;//logger
		throw (ResponseHandlerError(req->getVersion() + " 502 Bad Gateway", 502));
	}
	else
		response_header += CRLF;
}

void ResponseHandler::AppendCgiOutput(const std::string &buffer)
{
	try {
		CgiObj.ParseOutputBuffer(buffer);
	}
	catch (std::runtime_error &ex){
		CgiObj.KillChild();
		throw (ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));
	}
	catch (CgiHandler::BadCgiOutput &ex){
		DeleteCgiTargetFile();
		throw (ResponseHandlerError(req->getVersion() + " 502 Bad Gateway", 502));
	}
    if (ReachedCgiBodyPhase()){
		GenerateHeaderFromCgiData();
		response_body = CgiObj.GetPreservedBody();
    }
}

void ResponseHandler::SetTargetFileForCgi(int id)
{
	std::string filename = TMP_FILE_PREFIX + NumtoString(id);

	cgi_tmpfile_id = id;
	CgiObj.PreBodyPhraseChecks();
	int fd = open(filename.c_str(), O_CREAT, 0644);
	close(fd);
	target_file = new std::fstream(filename.c_str(), std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
	if (!target_file || fd == -1 || !target_file->is_open()){
		CgiObj.KillChild();
		throw (ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));}
	*target_file << response_body;
	cgi_buffer_size += response_body.size();
	if (target_file->bad() || target_file->fail()){
		DeleteCgiTargetFile();		
		CgiObj.KillChild();
		throw (ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));
	}
	response_body.clear();
}

void	ResponseHandler::DeleteCgiTargetFile()
{
	if (cgi_tmpfile_id == -1)
		return ;
	std::string filename = TMP_FILE_PREFIX + NumtoString(cgi_tmpfile_id);
	std::remove(filename.c_str());
}

void ResponseHandler::AppendBufferToTmpFile(const std::string &buf)
{
	if (!target_file || !target_file->is_open()){
		CgiObj.KillChild();
		DeleteCgiTargetFile();
		throw (ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));
	}
	else if (CgiObj.GetContentLength() != -1 && cgi_buffer_size + buf.size() > (unsigned)CgiObj.GetContentLength()){
		DeleteCgiTargetFile();		
		CgiObj.KillChild();
		// std::cout << "current content-lenght passed returend one" << std::endl;//logger
		throw (ResponseHandlerError(req->getVersion() + " 502 Bad Gateway", 502));
	}
	target_file->write(buf.c_str(), buf.size());
	if (target_file->fail() || target_file->bad()){
		DeleteCgiTargetFile();
		CgiObj.KillChild();
		throw (ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));
	}
	cgi_buffer_size += buf.size();
}
