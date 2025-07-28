#include "ServerManager.hpp"

void	Client::buildResponse() {
	_responseHandler->Run(_httpRequest);

	if (!_responseHandler->GetTargetFilePtr()) {
		// std::cout << "     =====>>>  Build Response for : {" << _socket.getFd() << "} <<<===== \n";

		_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
			// std::cout << "got full response (header + body)\n";
		setFullResponseFlag(FULL_RESPONSE_READY);
	}
	else {
		if (_responseHandler->IsPost()) {
			// std::cout << "     =====>>>  write body to target file <<<===== \n";

			_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
			_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
			// std::cout << "got full response (header + body)\n";
			
			std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
			ss >> _contentLength;
			// std::cout << "content length: " << _contentLength << "\n";
		}
		else {

			// std::cout << "    =======>>> Read data from target file to send <<<=====\n";

			_responseHolder = _responseHandler->GetResponseHeader();
			// std::cout << "Got only Response Header\n";
			_responseHeaderFlag = RESPONSE_HEADER_READY;
		}
	}
}