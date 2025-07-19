#include "ServerManager.hpp"

static std::pair<std::string, std::string> getResponseString(void) {

	std::pair<std::string, std::string> response;

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

		response.first = responseHeader;
		response.second = responseBody;

		return response;
}


void	Client::buildResponse() {
	// std::cout << "       ====>>> default Response <<<=====\n";
		
	std::pair<std::string, std::string> response = getResponseString();
	_requestHeaderPart = response.first;
	_requestBodyPart = response.second;
	_responseSize = _requestHeaderPart.size() + _requestBodyPart.size();
	_availableResponseBytes = _responseSize;
	// printRequestAndResponse("RESPONSE", _requestHeaderPart + _requestBodyPart);
}

// void	Client::buildResponse() {
// 	_responseHandler->Run(_httpRequest);


// 	if (!_responseHandler->GetTargetFilePtr()) {
// 		std::cout << "     =====>>>  Build Response (1) <<<===== \n";
// 		_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
// 		std::cout << "response has been created\n";
// 		_responseSize = _responseHolder.size();
// 		// _availableResponseBytes == _responseHolder.size();
// 		setAvailableResponseBytes(_responseSize);
// 		// std::cout << "size ==> " << _responseHolder.size() << "\n";
// 		// exit(0);
// 	}
// 	else {
// 		if (_responseHandler->IsPost()) {
// 			std::cout << "     =====>>>  write body to target file <<<===== \n";
// 			exit(0);
			
// 		}
// 		else {

// 			// std::cout << "       ====>>> default Response <<<=====\n";
			
// 			std::pair<std::string, std::string> response = getResponseString();
// 			_requestHeaderPart = response.first;
// 			_requestBodyPart = response.second;
// 			_responseSize = _requestHeaderPart.size() + _requestBodyPart.size();
// 			_availableResponseBytes = _responseSize;

// 			// std::cout << ":";
// 			printRequestAndResponse("RESPONSE", _requestHeaderPart + _requestBodyPart);
// 		}
// 	}
// }