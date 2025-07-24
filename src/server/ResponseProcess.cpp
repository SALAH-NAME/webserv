#include "ServerManager.hpp"

static std::string getResponseString(void) {

	std::string response;

	std::string responseBody =
			"<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"    <meta charset=\"UTF-8\">\n"
			"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
			"    <title>Welcome!</title>\n"
			"</head>\n"
			"<body style=\"font-family: sans-serif; background-color: #f4f4f4; margin: 40px; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); text-align: center;\">\n"
			"    <h1 style=\"color: #333; margin-bottom: 20px;\">Hello from My Server!</h1>\n"
			"    <p style=\"color: #666; line-height: 1.6; margin-bottom: 15px;\">You've reached a basic HTML page served by my simple web server.</p>\n"
			"    <p style=\"color: #007bff; font-weight: bold; margin-bottom: 15px;\">Enjoy the simplicity!</p>\n"
			"</body>\n"
			"</html>";
		
		std::stringstream ss;
		ss << responseBody.length();
		std::string contentLength = ss.str();
		
		std::string responseHeader =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + contentLength + "\r\n"
			"\r\n";

		response = responseHeader + responseBody;

		return response;
}

void	Client::buildResponse() {
	// std::cout << "RUN" << std::endl;
	_responseHandler->Run(_httpRequest);

	if (!_responseHandler->GetTargetFilePtr()) {
		// std::cout << "     =====>>>  Build Response for : {" << _socket.getFd() << "} <<<===== \n";
		_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
		_responseSize = _responseHolder.size();
		setAvailableResponseBytes(_responseSize);
		setResponseInFlight(true);
	}
	else {
		if (_responseHandler->IsPost()) {
			// std::cout << "     =====>>>  write body to target file <<<===== \n";
			_shouldTransferBody = TRANSFER_BODY_ON;
			_responseHolder = _responseHandler->GetResponseHeader();
			_responseSize = _responseHolder.size();
			setAvailableResponseBytes(_responseSize);
			// printRequestAndResponse("Response", _responseHolder);exit(0);
			std::stringstream ss(_httpRequest.getHeaders()["Content-Length"]);
			ss >> _contentLength;
			// std::cout << "Body Content-Length ==> " << _contentLength << "\n";
		}
		else {
			// std::cout << "       ====>>> default Response for : {" << _socket.getFd() << "} <<<=====\n";
			_responseHolder = getResponseString();
			_responseSize = _responseHolder.size();
			_availableResponseBytes = _responseSize;
			// printRequestAndResponse("RESPONSE", _requestHeaderPart + _requestBodyPart);
			setResponseInFlight(true);
		}
	}
}