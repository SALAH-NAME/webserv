#include "ServerManager.hpp"

void	Client::generateDynamicResponse() {
	_isCgiRequired = CGI_REQUIRED;
	// std::cout << "Is CGI required: " << _isCgiRequired << std::endl;

	_CGI_OutPipeFD = _responseHandler->GetCgiOutPipe().getReadFd();
	struct epoll_event					_event;
	_event.events = EPOLLIN | EPOLLHUP | EPOLLET;
	_event.data.fd = _CGI_OutPipeFD;
	
	try {
		if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _CGI_OutPipeFD, &_event) == -1)
			throw std::string("epoll_ctl failed");
		std::cout << "Pipe fd: " << _CGI_OutPipeFD << " Added successfully to epoll: " << _epfd << "\n";
	}
	catch(std::string e)
	{
		std::cout << e << "\n";
		return ;
	}	

	if (_responseHandler->IsPost()) {

		// std::cout << " ===>> CGI POST <<===\n";
		std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
			ss >> _contentLength;
		_CGI_InPipeFD = _responseHandler->GetCgiInPipe().getWriteFd();
		_event.events = EPOLLOUT | EPOLLET;
		_event.data.fd = _CGI_InPipeFD;
		try {
			if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _CGI_InPipeFD, &_event) == -1)
				throw std::string("epoll_ctl failed");
			// std::cout << "CGI Input Pipe fd: " << _CGI_InPipeFD << " Added successfully to epoll: " << _epfd << "\n";

			_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
			_pipeBodyToCgi = PIPE_TO_CGI;
			
		}
		catch(std::string e)
		{
			std::cout << e << "\n";
			return ;
		}
		
		if (_pendingRequestDataHolder.size()) {
			if (_isChunked)
				_state = ValidateChunkSize;
			else
				_state = ExtractingBody;
		}
		else
			_state = ReceivingData;
	}
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

			if (_isChunked)
				_contentLength = -1;
			else {
				std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
				ss >> _contentLength;
				// std::cout << "Body Content-Length ==> " << _contentLength << "\n
			}

			if (!_isChunked && !_contentLength)
				_state = UploadingToFile;
			else if (_requestDataPreloadedFlag == REQUEST_DATA_PRELOADED_ON) {
				if (_isChunked)
					_state = ValidateChunkSize;
				else
					_state = ExtractingBody;
			}
			else
				_state = ReceivingData;

			_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_ON;
			_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
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
	std::map<std::string, std::string> headers = _httpRequest.getHeaders();
	if (headers.find("host") != headers.end())
		_responseHandler->SetServerConf(getMatchingServerConfig(_allServersConfig, _httpRequest.getHeaders()["host"]), _clientInfos);
	else
		_responseHandler->SetServerConf(getMatchingServerConfig(_allServersConfig, ""), _clientInfos);
	
	_responseHandler->Run(_httpRequest);
	
	
	_isChunked = _httpRequest.isCunked();

	// std::cout << "is Chunked: " << _isChunked << "\n";
	// std::cout << "is CGI required: " << _responseHandler->RequireCgi() << "\n";

	if (_responseHandler->RequireCgi())
		generateDynamicResponse();
	else
		generateStaticResponse();
}
