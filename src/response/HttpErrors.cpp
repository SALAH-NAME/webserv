#include "ResponseHandler.hpp"

void ResponseHandler::CheckForInitialErrors(HttpRequest &req)
{
	std::map <std::string, std::string> headers = req.getHeaders();

	if (req.getVersion() != "HTTP/1.1")// using a different http version
        throw (ResponseHandlerError("HTTP/1.1 505 HTTP Version Not Supported", 505));
    if (headers.find("Host") == headers.end())// a request with no host header
        throw (ResponseHandlerError("HTTP/1.1 400 Bad Request", 400));
    try {stringToHttpMethod(req.getMethod());}
    catch (std::invalid_argument &ex){//    using a method other than GET, POST and DELETE 
        throw (ResponseHandlerError("HTTP/1.1 405 Not Allowed", 405));}
}

void	ResponseHandler::GenerateErrorPage(const std::string &status_line)
{
	response_body =
		"<html>\n<head><title>"+ status_line +"</title></head>\n"
		"<body>\n<center><h1>"+ status_line +"</h1></center>\n"
		"<hr><center>"+ SRV_NAME +"</center>\n"
		"</body>\n</html>\n";
	SetResponseHeader(status_line, response_body.size(), false);
}

void	ResponseHandler::LoadErrorPage(const std::string &status_line, int status_code)
{
	std::string error_page = conf.getErrorPage(status_code);
	
	if (error_page == "" || access(error_page.c_str(), R_OK) != 0)
		GenerateErrorPage(status_line);
	else
		LoadStaticFile(error_page, status_line);
}

ResponseHandler::ResponseHandlerError::ResponseHandlerError(const std::string &Errmsg, int statusCode) : error(Errmsg){
	status_code =statusCode;
}

const char *ResponseHandler::ResponseHandlerError::what()throw() {return error.c_str();}

int ResponseHandler::ResponseHandlerError::getStatusCode(){return status_code;}

ResponseHandler::ResponseHandlerError::~ResponseHandlerError() throw(){}
