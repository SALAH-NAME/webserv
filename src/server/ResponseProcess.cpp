#include "ServerManager.hpp"

void	Client::generateDynamicResponse() {
	_isCgiRequired = ON;
	_CGI_OutPipeFD = _responseHandler->GetCgiOutPipe().getReadFd();
	
	try {
		addSocketToEpoll(_epfd, _CGI_OutPipeFD, (EPOLLIN | EPOLLHUP | EPOLLERR));
	}
	catch(std::runtime_error& e)
	{
		close(_CGI_OutPipeFD);
		_CGI_OutPipeFD = -1;
		_state = DefaultState;
		throw e.what();
	}

	if (_responseHandler->IsPost()) {
		std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
			ss >> _contentLength;
		_CGI_InPipeFD = _responseHandler->GetCgiInPipe().getWriteFd();

		try {
			addSocketToEpoll(_epfd, _CGI_InPipeFD, (EPOLLOUT | EPOLLHUP | EPOLLERR)); 
			_InputState = INPUT_BODY_READY;
			_pipeBodyToCgi = ON;
		} catch(std::runtime_error& e)
		{
			close(_CGI_InPipeFD);
			_CGI_InPipeFD = -1;
			_state = DefaultState;
			throw e.what();
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
		_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
		setFullResponseFlag(ON);
	}
	else {
		if (_responseHandler->IsPost()) {
			if (_isChunked)
				_contentLength = -1;
			else {
				std::stringstream ss(_httpRequest.getHeaders()["content-length"]);
				ss >> _contentLength;
			}

			if (!_isChunked && !_contentLength)
				_state = UploadingToFile;
			else if (_requestDataPreloadedFlag == ON) {
				if (_isChunked)
					_state = ValidateChunkSize;
				else
					_state = ExtractingBody;
			}
			else
				_state = ReceivingData;

			_InputState = INPUT_BODY_READY;
			_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
		}
		else {
			_responseHolder = _responseHandler->GetResponseHeader();
			_responseHeaderFlag = ON;
		}
	}
}

void	Client::buildResponse() {
	_correctServerConfig = getMatchingServerConfig(_allServersConfig, _httpRequest, _clientInfos);
	_responseHandler->SetServerConf(_correctServerConfig, _clientInfos);
	_isChunked = _httpRequest.isCunked();
	
	_responseHandler->Run(_httpRequest);
	
	modifyEpollEvents(_epfd, _socket.getFd(), (EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR));

	if (_responseHandler->RequireCgi())
		generateDynamicResponse();
	else
		generateStaticResponse();
}
