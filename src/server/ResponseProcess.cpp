#include "ServerManager.hpp"

void	Client::extractBodyFromPendingRequestHolder() {
	size_t	bytesToExtract;

	std::cout << " ****************************************************** \n";
	std::cout << " ## Pending Request Data Holder size  ==> " << _pendingRequestDataHolder.size() << "\n";

	if (_pendingRequestDataHolder.size() <= _contentLength) {
		bytesToExtract = _pendingRequestDataHolder.size();
		// if (bytesToExtract < _contentLength)
		// 	_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
		// else
		// _incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
		_requestDataPreloadedFlag = REQUEST_DATA_PRELOADED_OFF;
	}
	else {
		bytesToExtract = _contentLength;
		// _incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
		_requestDataPreloadedFlag = REQUEST_DATA_PRELOADED_ON;
	}

	_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
	_bodyDataPreloadedFlag = BODY_DATA_PRELOADED_ON;
	_isRequestBodyWritable = WRITABLE;

	_requestBodyPart = _pendingRequestDataHolder.substr(0, bytesToExtract);
	_pendingRequestDataHolder = _pendingRequestDataHolder.substr(bytesToExtract);
	
	std::cout << " ## Request Data Preloaded Flag       ==> " << _requestDataPreloadedFlag << "\n";
	if (_requestDataPreloadedFlag)
		std::cout << " ## Pending Request Data size         ==> " << _pendingRequestDataHolder.size() << "\n\n"; 
	else
		std::cout << "\n";

	std::cout << " ## Incoming Body Data Detected Flag  ==> " << _incomingBodyDataDetectedFlag << "\n";
	std::cout << " ## Dody Data Preloaded Flag          ==> " << _bodyDataPreloadedFlag << "\n";
	std::cout << " ## Is Request Body Writable          ==> " << _isRequestBodyWritable << "\n";
	std::cout << " ## Request Body Part size            ==> " << _requestBodyPart.size() << "\n";
	std::cout << " ****************************************************** \n";

	// exit(0);
}

void	Client::buildResponse() {

	std::cout << "keep-alive: " << _httpRequest.getHeaders()["connection"] << "\n";
	if (_httpRequest.getHeaders()["connection"] == "keep-alive")
		_isKeepAlive = ENABLE_KEEP_ALIVE;
	else if (_httpRequest.getHeaders()["connection"] == "close")
		_isKeepAlive = DISABLE_KEEP_ALIVE;

	_responseHandler->Run(_httpRequest);


	if (!_responseHandler->GetTargetFilePtr()) {
		std::cout << "     =====>>>  Build Response for : {" << _socket.getFd() << "} <<<===== \n";

		_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
			// std::cout << "got full response (header + body)\n";
		setFullResponseFlag(FULL_RESPONSE_READY);
	}
	else {
		if (_responseHandler->IsPost()) {
			std::cout << "     =====>>>  write body to target file <<<===== \n";

			std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
			ss >> _contentLength;
			std::cout << "Body Content-Length ==> " << _contentLength << "\n";

			if (_requestDataPreloadedFlag == REQUEST_DATA_PRELOADED_ON)
				extractBodyFromPendingRequestHolder();
			// _incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
			_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
			// std::cout << "got full response (header + body)\n";
			
		}
		else {

			// std::cout << "    =======>>> Read data from target file to send <<<=====\n";

			_responseHolder = _responseHandler->GetResponseHeader();
			// std::cout << "Got only Response Header\n";
			_responseHeaderFlag = RESPONSE_HEADER_READY;
		}
	}
}