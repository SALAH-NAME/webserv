#include "ServerManager.hpp"

void	Client::buildResponse() {
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
			// printRequestAndResponse("Response", _responseHolder);
			std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
			ss >> _contentLength;
			std::cout << "Body Content-Length ==> " << _contentLength << "\n";
		}
		else {

			// std::cout << "    =======>>> Read data from target file to send <<<=====\n";
			_responseHolder = _responseHandler->GetResponseHeader();
			_responseSize = _responseHolder.size();
			setAvailableResponseBytes(_responseSize);

			// printRequestAndResponse("Response header", _responseHolder);

			analyzeResponseHolder();
		}
	}
}