
#include "Client.hpp"

Client::Client(Socket sock, const ServerConfig& conf, const std::vector<ServerConfig>& allServersConfig, int epfd, ClientInfos clientInfos) : _socket(sock)
											, _epfd(epfd)
											, _conf(conf)
											, _allServersConfig(allServersConfig)
											, _clientInfos(clientInfos)
											, _CGI_OutPipeFD(-1)
											, _CGI_InPipeFD(-1)
											, _state(DefaultState)
											, _lastTimeConnection(std::time(NULL))
											, _contentLength(0)
											, _chunkBodySize(-1)
											, _isChunked(NOT_CHUNKED)
											, _uploadedBytes(0)
											, _responseHandler(new ResponseHandler())
											, _incomingHeaderDataDetectedFlag(INCOMING_HEADER_DATA_OFF)
											, _incomingBodyDataDetectedFlag(INCOMING_BODY_DATA_OFF)
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
											, _isPipeClosedByPeer(PIPE_IS_NOT_CLOSED)
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
									, _conf(other._conf)
									, _allServersConfig(other._allServersConfig)
									, _clientInfos(other._clientInfos)
									, _CGI_OutPipeFD(other._CGI_OutPipeFD)
									, _CGI_InPipeFD(other._CGI_InPipeFD)
									, _state(other._state)
									, _lastTimeConnection(other._lastTimeConnection)
									, _contentLength(other._contentLength)
									, _chunkBodySize(other._chunkBodySize)
									, _isChunked(other._isChunked)
									, _uploadedBytes(other._uploadedBytes)
									, _responseHandler(other._responseHandler)
									, _incomingHeaderDataDetectedFlag(other._incomingHeaderDataDetectedFlag)
									, _incomingBodyDataDetectedFlag(other._incomingBodyDataDetectedFlag)
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
									, _isPipeClosedByPeer(other._isPipeClosedByPeer)
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

int	Client::getCGI_OutpipeFD(void) {
	return _CGI_OutPipeFD;
}

int	Client::getCGI_InpipeFD(void) {
	return _CGI_InPipeFD;
}

PostMethodProcessingState&	Client::getState(void) {
	return _state;
}

time_t Client::getLastConnectionTime(void)
{
	return _lastTimeConnection;
}

bool	Client::getIncomingHeaderDataDetectedFlag(void) {
	return _incomingHeaderDataDetectedFlag;
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

bool	Client::getIncomingBodyDataDetectedFlag(void) {
	return _incomingBodyDataDetectedFlag;
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

int	Client::getIsPipeClosedByPeer(void) {
	return _isPipeClosedByPeer;
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

void Client::appendToBodyPart(const std::string &bodyData)
{
	_requestBodyPart += bodyData;
}

void	Client::resetLastConnectionTime(void){
	_lastTimeConnection = std::time(NULL);
}

void	Client::setIncomingHeaderDataDetectedFlag(int mode) {
	_incomingHeaderDataDetectedFlag = mode;
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

void	Client::setIncomingBodyDataDetectedFlag(bool value) {
	_incomingBodyDataDetectedFlag = value;
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

void	Client::setIsPipeClosedByPeer(int	value) {
	_isPipeClosedByPeer = value;
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

void    Client::closeAndDeregisterPipe(int pipeFD) {
    std::cout << "============================================================\n";
    epoll_ctl(_epfd, EPOLL_CTL_DEL, pipeFD, NULL);
    std::cout << "  ## Removed Pipe fd: " << pipeFD << " from epoll  ## \n";
    close(pipeFD);
    pipeFD = -1;
    std::cout << "  ## closed Pipe fd: " << pipeFD << "              ## \n";
    std::cout << "============================================================\n\n";
    
}

void    Client::CgiExceptionHandler() {
    if (_CGI_OutPipeFD != -1) {
        closeAndDeregisterPipe(_CGI_OutPipeFD);
        _isPipeClosedByPeer = PIPE_IS_NOT_CLOSED;
    }
    if (_CGI_InPipeFD != -1) {
        closeAndDeregisterPipe(_CGI_InPipeFD);
        _incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
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

void				Client::printClientStatus(void) {
	std::cout << "\n ------------------------------------------------------\n";
	std::cout << "  ## _CGI_OutPipeFD: " <<  _CGI_OutPipeFD << " ## \n";
	std::cout << "  ##  _requestHeaderPart size : " << _requestHeaderPart.size() << "  ## \n";
	std::cout << "  ##  _requestBodyPart size : " << _requestBodyPart.size() << "  ## \n";
	std::cout << "  ##  _responseHolder size : " << _responseHolder.size() << "  ## \n";
	std::cout << "  ##  _pendingRequestDataHolder size : " << _pendingRequestDataHolder.size() << "  ## \n";
	std::cout << "  ##  _contentLength : " << _contentLength << "  ## \n";
	std::cout << "  ##  _uploadedBytes : " << _uploadedBytes << "  ## \n";
	std::cout << "  ##  _responseSent : " << _responseSent << "  ## \n";
	std::cout << "--------------------------------------------------------\n\n";
}

void	Client::resetAttributes(void) {
	std::memset((void*)&_clientInfos, 0, sizeof(ClientInfos));
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
		_incomingHeaderDataDetectedFlag = INCOMING_HEADER_DATA_ON;
	else
		_incomingHeaderDataDetectedFlag =  INCOMING_HEADER_DATA_OFF;
	_incomingBodyDataDetectedFlag =  INCOMING_BODY_DATA_OFF;
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
	_isPipeClosedByPeer =  PIPE_IS_NOT_CLOSED;
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
