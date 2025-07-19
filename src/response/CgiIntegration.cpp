#include "ResponseHandler.hpp"

Pipe &ResponseHandler::GetCgiInPipe(){return (CgiObj.GetInPipe());}

Pipe &ResponseHandler::GetCgiOutPipe(){return (CgiObj.GetOutPipe());}

pid_t ResponseHandler::GetCgiChildPid(){return (CgiObj.GetChildPid());}

std::string	ResponseHandler::GenerateCgiStatusLine()
{
	int 		status_code = CgiObj.GetStatusCode();
	std::string	reason_phrase = CgiObj.GetReasonPhrase();

	if (status_code == 0)
		return ("HTTP/1.1 200 OK");
	if (!reason_phrase.empty())
		return ("HTTP/1.1 " + NumtoString(status_code) + ' ' + reason_phrase);
	else if (status_phrases.find(status_code) != status_phrases.end())
		return ("HTTP/1.1 " + NumtoString(status_code) + ' ' + status_phrases[status_code]);
	else
		return "Unknown";
}

void ResponseHandler::GenerateHeaderFromCgiData()
{
	std::map<std::string, std::string> &headers = CgiObj.GetOutputHeaders();
	bool has_date = headers.find("Date") != headers.end();
	bool has_name = headers.find("Server") != headers.end();
	response_header += GenerateCgiStatusLine() + CRLF;
	response_header += has_date ? "" : GenerateTimeStamp() + CRLF;
	response_header += has_name ? "" : SRV_NAME + std::string(CRLF); 
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		response_header += it->first + ":" + it->second;
}

void    ResponseHandler::AppendToCgiOutput(const std::string &buffer)
{
	try {
		CgiObj.ParseOutputBuffer(buffer);
	}
	catch (std::runtime_error &ex){
		throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
	}
	catch (CgiHandler::BadCgiOutput &ex){
		throw (ResponseHandlerError("HTTP/1.1 502 Bad Gateway", 502));
	}
    if (CgiObj.ReachedBodyPhase()){
		GenerateHeaderFromCgiData();
		response_body = CgiObj.GetPreservedBody();
    }
}

bool    ResponseHandler::checkCgiTimeOut()
{
    return (std::time(NULL) - CgiObj.GetExecutionStartTime()
                >= loc_config->getCgiTimeout());
}

