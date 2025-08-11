#include "ResponseHandler.hpp"

void ResponseHandler::InitialRequestCheck()
{
	if (!req->isValid()){
		std::string full_status_line =	req->getVersion() + " " + NumtoString(req->getStatusCode()) +
											' ' + req->getErrorMsg();
		keep_alive = false;
		throw (ResponseHandlerError(full_status_line, req->getStatusCode()));
	}
		return ;
}

void	ResponseHandler::GenerateErrorPage( const std::string &status_line)
{
	response_body =
		"<html>\n<head><title>"+ status_line +"</title></head>\n"
		"<body>\n<center><h1>"+ status_line +"</h1></center>\n"
		"<hr><center>"+ SRV_NAME +"</center>\n"
		"</body>\n</html>\n";
	SetResponseHeader(status_line, response_body.size(), false);
}

void	ResponseHandler::LoadErrorPage( const std::string &status_line, int status_code)
{
	RefreshData();
	std::string error_page = conf.getErrorPage(status_code);

	if (error_page == "" || access(error_page.c_str(), R_OK) != 0)
		GenerateErrorPage(status_line);
	else
		LoadStaticFile(error_page, status_line);
}

ResponseHandler::ResponseHandlerError::ResponseHandlerError(const std::string &Errmsg, int statusCode) : error(Errmsg){
	status_code =statusCode;
}

const char *ResponseHandler::ResponseHandlerError::what()const throw() {return error.c_str();}

int ResponseHandler::ResponseHandlerError::getStatusCode(){return status_code;}

ResponseHandler::ResponseHandlerError::~ResponseHandlerError() throw(){}
