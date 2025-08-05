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
	return (GetCgiChildExitStatus() == -1);
}

bool ResponseHandler::CheckCgiTimeOut()
{
	if (IsCgiChildRunning())
		return false;
	return (std::time(NULL) - CgiObj.GetExecutionStartTime()
                >= loc_config->getCgiTimeout());
}

int ResponseHandler::GetCgiChildExitStatus()//returns -1 if cgi child still running 
{
	int exit_status;
	if (waitpid(GetCgiChildPid(), &exit_status, WNOHANG) == GetCgiChildPid())
		return exit_status;
	else
		return -1;
}

std::string	ResponseHandler::GenerateCgiStatusLine()
{
	int 		status_code = CgiObj.GetStatusCode();
	std::string	reason_phrase = CgiObj.GetReasonPhrase();

	if (status_code == 0)
		return ("HTTP/1.1 200 OK");
	if (!reason_phrase.empty())
		return ("HTTP/1.1 " + NumtoString(status_code) + reason_phrase);
	else if (status_phrases.find(status_code) != status_phrases.end())
		return ("HTTP/1.1 " + NumtoString(status_code) + ' ' + status_phrases[status_code]);
	else
		return "HTTP/1.1 " + NumtoString(status_code) + ' ' + "Unknown";
}

void ResponseHandler::GenerateHeaderFromCgiData()
{
	std::map<std::string, std::string> &headers = CgiObj.GetOutputHeaders();
	std::vector<std::string> &extra_cookies = CgiObj.GetExtraCookieValues();

	bool has_date = headers.find("date") != headers.end();
	bool has_name = headers.find("server") != headers.end();//use server and date
	response_header += GenerateCgiStatusLine() + CRLF;
	response_header += has_date ? "" : "Date: " + GenerateTimeStamp() + CRLF;
	response_header += has_name ? "" : "Server: " + std::string(SRV_NAME) + CRLF; 
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
	else if (cgi_buffer_size + response_body.size() != (unsigned)returned_length)
		throw (ResponseHandlerError("HTTP/1.1 502 Bad Gateway", 502));
	else
		response_header+=CRLF;
}

void ResponseHandler::AppendCgiOutput(const std::string &buffer)
{
	try {
		CgiObj.ParseOutputBuffer(buffer);
	}
	catch (std::runtime_error &ex){
		throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));}
	catch (CgiHandler::BadCgiOutput &ex){
		throw (ResponseHandlerError("HTTP/1.1 502 Bad Gateway", 502));}
    if (ReachedCgiBodyPhase()){
		GenerateHeaderFromCgiData();
		response_body = CgiObj.GetPreservedBody();
    }
}

void ResponseHandler::CheckCgiChildState() // use only if cgi is required
{
	int exit_status = GetCgiChildExitStatus();

	if (CheckCgiTimeOut()){
		CgiObj.KillChild();
		throw (ResponseHandlerError("HTTP/1.1 504 Gateway Timeout", 504));
	}
	else if (!IsCgiChildRunning() && exit_status != 0)
		throw (ResponseHandlerError("HTTP/1.1 502 Bad Gateway", 502));
}

void ResponseHandler::SetTargetFileForCgi(int count)
{
	std::string filename = TMP_FILE_PREFIX + NumtoString(count);
	if (target_file)
		delete target_file;
	int fd = open(filename.c_str(), O_CREAT, 0644);
	close(fd);
	target_file = new std::fstream(filename.c_str(), std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
	if (!target_file || fd == -1 || !target_file->is_open())
		throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
	*target_file << response_body;
	cgi_buffer_size += response_body.size();
	if (target_file->bad() || target_file->fail())
		throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
	response_body.clear();
}

void ResponseHandler::AppendBufferToTmpFile(const std::string &buf)
{
	if (!target_file || !target_file->is_open())
        throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
	else if (CgiObj.GetContentLength() != -1 && cgi_buffer_size + buf.size() > (unsigned)CgiObj.GetContentLength())
		throw (ResponseHandlerError("HTTP/1.1 502 Bad Gateway", 502));
	target_file->write(buf.c_str(), buf.size());
	if (target_file->fail() || target_file->bad())
		throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
	cgi_buffer_size += buf.size();
}
