#include "ServerManager.hpp"

void	Client::extractBodyFromPendingRequestHolder() {
	size_t	bytesToExtract;
	if (_pendingRequestDataHolder.size() < _contentLength) {
		bytesToExtract = _pendingRequestDataHolder.size();
		_requestDataPreloadedFlag = REQUEST_DATA_PRELOADED_OFF;
		_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
	}
	else if (_pendingRequestDataHolder.size() > _contentLength) {
		bytesToExtract = _contentLength;
		_requestDataPreloadedFlag = REQUEST_DATA_PRELOADED_ON;
		_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
	}
	else {
		bytesToExtract = _contentLength;
		_requestDataPreloadedFlag = REQUEST_DATA_PRELOADED_OFF;
		_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
	}

	_bodyDataPreloadedFlag = BODY_DATA_PRELOADED_ON;
	_isRequestBodyWritable = WRITABLE;

	_requestBodyPart = _pendingRequestDataHolder.substr(0, bytesToExtract);
	_pendingRequestDataHolder = _pendingRequestDataHolder.substr(bytesToExtract);
}

void	Client::generateDynamicResponse() {
	_CGI_pipeFD = _responseHandler->GetCgiOutPipe().getReadFd();
	struct epoll_event					_event;
	_event.events = EPOLLIN | EPOLLHUP | EPOLLET;
	_event.data.fd = _CGI_pipeFD;
	
	try {
		if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _CGI_pipeFD, &_event) == -1)
			throw std::string("epoll_ctl failed");
	}
	catch(std::string e)
	{
		std::cout << e << "\n";
		return ;
	}

	_isCgiRequired = CGI_REQUIRED;
	std::cout << "Pipe fd: " << _CGI_pipeFD << " Added successfully to epoll: " << _epfd << "\n";
	std::cout << "Is CGI required: " << _isCgiRequired << std::endl;

}

void	Client::generateStaticResponse() {

	if (!_responseHandler->GetTargetFilePtr()) {
		// std::cout << "     =====>>>  No target file needed : {" << _socket.getFd() << "} <<<===== \n";
		_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
			// std::cout << "got full response (header + body)\n";
		setFullResponseFlag(FULL_RESPONSE_READY);
	}
	else {
		if (_responseHandler->IsPost()) {
			// std::cout << "     =====>>>  write body to target file <<<===== \n";

			std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
			ss >> _contentLength;
			// std::cout << "Body Content-Length ==> " << _contentLength << "\n";

			if (_requestDataPreloadedFlag == REQUEST_DATA_PRELOADED_ON)
				extractBodyFromPendingRequestHolder();
			else
				_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
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

void	Client::buildResponse() {
	_responseHandler->Run(_httpRequest);

	if (_responseHandler->RequireCgi())
		generateDynamicResponse();
	else
		generateStaticResponse();
}
