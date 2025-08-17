
#include "Client.hpp"

Client::Client(ServerManager* serverManagerPtr, Socket sock, const std::vector<ServerConfig>& allServersConfig, int epfd, ClientInfos clientInfos) : _socket(sock)
											, _epfd(epfd)
											, _allServersConfig(allServersConfig)
											, _clientInfos(clientInfos)
											, _serverManagerPtr(serverManagerPtr)
											, _CGI_OutPipeFD(-1)
											, _CGI_InPipeFD(-1)
											, _state(DefaultState)
											, _InputState(INPUT_NONE)
											, _lastTimeConnection(std::time(NULL))
											, _contentLength(0)
											, _chunkBodySize(-1)
											, _isChunked(NOT_CHUNKED)
											, _uploadedBytes(0)
											, _responseHandler(new ResponseHandler())
											, _responseHeaderFlag(RESPONSE_HEADER_NOT_READY)
											, _responseBodyFlag(RESPONSE_BODY_NOT_READY)
											, _fullResponseFlag(FULL_RESPONSE_NOT_READY)
											, _generateInProcess(GENERATE_RESPONSE_OFF)
											, _isResponseBodySendable(NOT_SENDABLE)
											, _isRequestBodyWritable(NOT_WRITABLE)
											, _bodyDataPreloadedFlag(BODY_DATA_PRELOADED_OFF)
											, _requestDataPreloadedFlag(REQUEST_DATA_PRELOADED_OFF)
											, _pendingHeaderFlag(false)
											, _isCgiRequired(CGI_IS_NOT_REQUIRED)
											, _isPipeReadable(PIPE_IS_NOT_READABLE)
											, _pipeReadComplete(READ_PIPE_NOT_COMPLETE)
											, _setTargetFile(false)
											, _responseSent(NOT_SENT)
											, _isOutputAvailable(NOT_AVAILABLE)
											, _isCgiInputAvailable(NOT_AVAILABLE)
											, _pipeBodyToCgi(NO_PIPE)
											, _isBodyTooLarge(false)
{}

Client::Client(const Client& other) : _socket(other._socket)
									, _epfd(other._epfd)
									, _allServersConfig(other._allServersConfig)
									, _correctServerConfig(other._correctServerConfig)
									, _clientInfos(other._clientInfos)
									, _serverManagerPtr(other._serverManagerPtr)
									, _CGI_OutPipeFD(other._CGI_OutPipeFD)
									, _CGI_InPipeFD(other._CGI_InPipeFD)
									, _state(other._state)
									, _InputState(other._InputState)
									, _lastTimeConnection(other._lastTimeConnection)
									, _contentLength(other._contentLength)
									, _chunkBodySize(other._chunkBodySize)
									, _isChunked(other._isChunked)
									, _uploadedBytes(other._uploadedBytes)
									, _responseHandler(other._responseHandler)
									, _responseHeaderFlag(other._responseHeaderFlag)
									, _responseBodyFlag(other._responseBodyFlag)
									, _fullResponseFlag(other._fullResponseFlag)
									, _generateInProcess(other._generateInProcess)
									, _isResponseBodySendable(other._isResponseBodySendable)
									, _isRequestBodyWritable(other._isRequestBodyWritable)
									, _bodyDataPreloadedFlag(other._bodyDataPreloadedFlag)
									, _requestDataPreloadedFlag(other._requestDataPreloadedFlag)
									, _pendingHeaderFlag(other._pendingHeaderFlag)
									, _isCgiRequired(other._isCgiRequired)
									, _isPipeReadable(other._isPipeReadable)
									, _pipeReadComplete(other._pipeReadComplete)
									, _setTargetFile(other._setTargetFile)
									, _responseSent(other._responseSent)
									, _isOutputAvailable(other._isOutputAvailable)
									, _isCgiInputAvailable(other._isCgiInputAvailable)
									, _pipeBodyToCgi(other._pipeBodyToCgi)
									, _isBodyTooLarge(other._isBodyTooLarge)
{
	const_cast<Client&> (other)._responseHandler = NULL;
}

Client::~Client() {
	if (_responseHandler) {
		delete _responseHandler;
	}
}

Socket &Client::getSocket()
{
	return _socket;
}

int	Client::getEpfd(void){
	return _epfd;
}

ServerConfig*	Client::getCorrectServerConfig(void) {
	return _correctServerConfig;
}

int	Client::getCGI_OutpipeFD(void) {
	return _CGI_OutPipeFD;
}

int	Client::getCGI_InpipeFD(void) {
	return _CGI_InPipeFD;
}

PostMethodProcessingState&	Client::getState(void) {
	return _state;
}

ClientInputState	Client::getInputState(void) {
	return _InputState;
}

time_t Client::getLastConnectionTime(void)
{
	return _lastTimeConnection;
}

std::string&	Client::getRequestBodyPart(void) {
	return _requestBodyPart;
}

ResponseHandler*	Client::getResponseHandler(void) {
	return _responseHandler;
}

std::string&	Client::getPendingRequestData(void) {
	return _pendingRequestDataHolder;
}

int Client::getBytesToSendNow(void)
{
	if (_responseHolder.size() >= BYTES_TO_SEND)
		return BYTES_TO_SEND;
	else
		return _responseHolder.size();
}

bool Client::getGenerateInProcess(void)
{
	return _generateInProcess;
}

HttpRequest &Client::getHttpRequest(void)
{
	return _httpRequest;
}

std::string &Client::getHeaderPart(void)
{
	return _requestHeaderPart;
}

std::string &Client::getBodyPart(void)
{
	return _requestBodyPart;
}

bool	Client::getIsRequestBodyWritable(void) {
	return _isRequestBodyWritable;
}

bool	Client::getIsCgiRequired(void) {
	return _isCgiRequired;
}

bool	Client::getIsPipeReadable(void) {
	return _isPipeReadable;
}

void Client::setResponseHeaderFlag(bool value)
{
	_responseHeaderFlag = value;
}

void	Client::setFullResponseFlag(bool value) {
	_fullResponseFlag = value;
}

bool Client::getResponseHeaderFlag(void)
{
	return _responseHeaderFlag;
}

bool	Client::getResponseBodyFlag(void) {
	return _responseBodyFlag;
}

bool	Client::getFullResponseFlag(void) {
	return _fullResponseFlag;
}

size_t	Client::getUploadedBytes(void) {
	return _uploadedBytes;
}

bool	Client::getBodyDataPreloadedFlag(void) {
	return _bodyDataPreloadedFlag;
}

bool	Client::getRequestDataPreloadedFlag(void) {
	return _requestDataPreloadedFlag;
}

size_t	Client::getContentLength(void) {
	return _contentLength;
}

std::string&		Client::getResponseHolder(void) {
	return _responseHolder;
}

bool	Client::getSetTargetFile(void) {
	return _setTargetFile;
}

void		Client::appendToHeaderPart(const std::string& headerData) {
	_requestHeaderPart += headerData;
}

bool	Client::getIsResponseBodySendable(void) {
	return _isResponseBodySendable;
}

bool	Client::getResponseSent(void) {
	return _responseSent;
}

bool	Client::getIsOutputAvailable(void) {
	return _isOutputAvailable;
}

bool	Client::getPipeBodyToCgi(void) {
	return _pipeBodyToCgi;
}

void	Client::setInputState(ClientInputState value) {
	_InputState = value;
}

void Client::appendToBodyPart(const std::string &bodyData)
{
	_requestBodyPart += bodyData;
}

void	Client::resetLastConnectionTime(void){
	_lastTimeConnection = std::time(NULL);
}

void Client::setGenerateResponseInProcess(bool value)
{
	_generateInProcess = value;
}

void	Client::setBodyDataPreloadedFlag(bool value) {
	_bodyDataPreloadedFlag = value;
}

void	Client::setRequestDataPreloadedFlag(bool value) {
	_requestDataPreloadedFlag = value;
}

void	Client::setPendingHeaderFlag(bool value) {
	_pendingHeaderFlag = value;
}

void	Client::setRequestBodyPart(std::string bodyData) {
	_requestBodyPart = bodyData;
}

void	Client::setPendingRequestData(std::string bodyData) {
	_pendingRequestDataHolder = bodyData;
}

void	Client::setUploadedBytes(size_t bytes) {
	_uploadedBytes = bytes;
}

void	Client::resetUploadedBytes(void) {
	_uploadedBytes = 0;
}

void	Client::setContentLength(int length) {
	_contentLength = length;
}

void	Client::resetContentLength(void) {
	_contentLength = 0;
}

void	Client::setHeaderPart(std::string HeaderData) {
	_requestHeaderPart = HeaderData;
}

void	Client::setResponseHolder(const std::string responseData) {
	_responseHolder = responseData;
}

void	Client::setIsRequestBodyWritable(bool value) {
	_isRequestBodyWritable = value;
}

void	Client::setIsPipeReadable(bool value) {
	_isPipeReadable = value;
}

void	Client::setIsCgiRequired(bool value) {
	_isCgiRequired = value;
}

void	Client::setPipeReadComplete(bool value) {
	_pipeReadComplete = value;
}

void	Client::setSetTargetFile(bool value) {
	_setTargetFile = value;
}

void	Client::setIsOutputAvailable(bool value) {
	_isOutputAvailable = value;
}

void	Client::setIsCgiInputAvailable(bool value) {
	_isCgiInputAvailable = value;
}

bool Client::parseRequest()
{
	try
	{
		_httpRequest.appendAndValidate(_requestHeaderPart);
		return _httpRequest.isValid();
	}
	catch (const HttpRequestException &ex)
	{
		return false;
	}
}

void Client::prinfRequestinfos(void)
{
	_httpRequest.printInfos();
}

void	Client::updateHeaderStateAfterSend(size_t sentBytes) {
	_responseHolder = _responseHolder.substr(sentBytes);
	if (!_responseHolder.size()) {
		if (_responseHeaderFlag == RESPONSE_HEADER_READY) {
			_responseHeaderFlag = RESPONSE_HEADER_NOT_READY;
			_responseBodyFlag = RESPONSE_BODY_READY;
			// std::cout << " ==> Sent Header Successfully <==\n";
		}
		else {
			_fullResponseFlag = FULL_RESPONSE_NOT_READY;
			_responseBodyFlag = RESPONSE_BODY_NOT_READY;
			_responseSent = SENT;
			// std::cout << " ==> Sent Full Response Successfully <==\n";
		}
	}
}

void	Client::readFileBody(void) {
	std::fstream *targetFile = _responseHandler->GetTargetFilePtr();

	char buffer[BYTES_TO_SEND+1];
	targetFile->read(buffer, BYTES_TO_SEND);
	ssize_t _bytesReadFromFile = targetFile->gcount();
	if (_bytesReadFromFile < 0)
		return ;
	// std::cout << "read bytes: " << _bytesReadFromFile << "\n";

	buffer[_bytesReadFromFile] = 0;
	_responseHolder = std::string(buffer, _bytesReadFromFile);
	_isResponseBodySendable = SENDABLE;
}

void	Client::sendFileBody(void) {
	ssize_t sentBytes;

	if (!_isOutputAvailable)
		return ;
	
	if (_responseHolder.size()) {
		sentBytes = _socket.send(_responseHolder.c_str(), _responseHolder.size(), MSG_NOSIGNAL);
		if (sentBytes <= 0)
			return ;
		resetLastConnectionTime();
	}
	// std::cout << "send bytes: " << sentBytes << "\n";
	
	if (_responseHolder.size() < BYTES_TO_SEND) {
		_isResponseBodySendable = NOT_SENDABLE;
		_responseBodyFlag = RESPONSE_BODY_NOT_READY;
		_responseSent = SENT;
		// std::cout << " ==>> Send response body Successfully \n";
		return ;
	}
	
	_isResponseBodySendable = NOT_SENDABLE;
}

void    Client::closeAndDeregisterPipe(bool pipe) {
    std::cout << "============================================================\n";
	int* pipePTR;
	if (pipe == IN_PIPE)
		pipePTR = &_CGI_InPipeFD;
	else
		pipePTR = &_CGI_OutPipeFD;

    epoll_ctl(_epfd, EPOLL_CTL_DEL, *pipePTR, NULL);
    std::cout << "  ## Removed Pipe fd: " << *pipePTR << " from epoll  ## \n";
    close(*pipePTR);
    *pipePTR = -1;
    std::cout << "  ## closed Pipe fd: " << *pipePTR << "              ## \n";
    std::cout << "============================================================\n\n";
    
}

void    Client::CgiExceptionHandler() {
    if (_CGI_OutPipeFD != -1) {
        closeAndDeregisterPipe(OUT_PIPE);
		_InputState = INPUT_NONE;
    }
    if (_CGI_InPipeFD != -1) {
        closeAndDeregisterPipe(IN_PIPE);
		_InputState = INPUT_NONE;
    }

    if (_responseHandler->GetTargetFilePtr()) {
        _responseHolder = _responseHandler->GetResponseHeader();
        _responseHeaderFlag = RESPONSE_HEADER_READY;
    }
    else {
        _responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
        _fullResponseFlag = FULL_RESPONSE_READY;
    }
}

void	Client::resetAttributes(void) {
	_CGI_OutPipeFD = -1;
	_CGI_InPipeFD = -1;
	_state = DefaultState;
	_lastTimeConnection =  std::time(NULL);
	_contentLength =  0;
	_chunkBodySize = -1;
	_isChunked = NOT_CHUNKED;
	_uploadedBytes =  0;
	_httpRequest.reset();
	delete _responseHandler;
	_responseHandler = new ResponseHandler();

	if (_requestDataPreloadedFlag || _pendingHeaderFlag)
		_InputState = INPUT_HEADER_READY;
	else
		_InputState = INPUT_NONE;

	_responseHeaderFlag =  RESPONSE_HEADER_NOT_READY;
	_responseBodyFlag =  RESPONSE_BODY_NOT_READY;
	_fullResponseFlag =  FULL_RESPONSE_NOT_READY;
	_generateInProcess =  GENERATE_RESPONSE_OFF;
	_isResponseBodySendable =  NOT_SENDABLE;
	_isRequestBodyWritable =  NOT_WRITABLE;
	_bodyDataPreloadedFlag =  BODY_DATA_PRELOADED_OFF;
	_pendingHeaderFlag = false;
	_isCgiRequired =  CGI_IS_NOT_REQUIRED;
	_isPipeReadable =  PIPE_IS_NOT_READABLE;
	_pipeReadComplete =  READ_PIPE_NOT_COMPLETE;
	_setTargetFile =  false;
	_responseSent = NOT_SENT;
	_isCgiInputAvailable = NOT_AVAILABLE;
	_pipeBodyToCgi = NO_PIPE;
	_isBodyTooLarge = false;

	_requestHeaderPart.clear();
	_requestBodyPart.clear();
	_responseHolder.clear();

	// std::cout << " ## RESETED ##\n";
}

void	Client::getBufferFromPendingData(char* buffer, ssize_t* readBytes) {
	size_t i;
	for (i = 0; i < _pendingRequestDataHolder.size(); i++) {
		buffer[i] = _pendingRequestDataHolder[i];
 	}
	buffer[i] = 0;
	*readBytes = i;

	_pendingRequestDataHolder.clear();
	_requestDataPreloadedFlag = REQUEST_DATA_PRELOADED_OFF;
}
