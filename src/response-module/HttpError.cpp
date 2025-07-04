#include "ResponseHandler.hpp"

void ResponseHandler::CheckForInitialErrors(Request &req)
{
	if (req.getHttpVersion() != "HTTP/1.1")// using a different http version
        throw (RequestError("HTTP/1.1 505 HTTP Version Not Supported", 505));
    if (req.getHeaders().find("Host") == req.getHeaders().end())// a request with no host header
        throw (RequestError("HTTP/1.1 400 Bad Request", 400));
    try {stringToHttpMethod(req.getMethod());}
    catch (std::invalid_argument){//    using a method other than GET, POST and DELETE 
        throw (RequestError("HTTP/1.1 405 Not Allowed", 405));}
}

void	ResponseHandler::GenerateErrorPage(const std::string &status_line, Request &req)
{
	response_body =
		"<html>\n<head><title>"+ status_line +"</title></head>\n"
		"<body>\n<center><h1>"+ status_line +"</h1></center>\n"
		"<hr><center>"+ SRV_NAME +"</center>\n"
		"</body>\n</html>\n";
	SetResponseHeader(req, status_line, response_body.size());
}

void	ResponseHandler::LoadErrorPage(const std::string &status_line, int status_code, Request &req)
{
	std::string error_page = conf.getErrorPage(status_code);
	
	if (error_page == "" || access(error_page.c_str(), R_OK)!= 0)
		GenerateErrorPage(status_line, req);
	
}

ResponseHandler::RequestError::RequestError(const std::string &Errmsg, int statusCode)
    : error(Errmsg) , status_code(statusCode){}

const char *ResponseHandler::RequestError::what(){return error.c_str();}

int ResponseHandler::RequestError::getStatusCode(){return status_code;}

