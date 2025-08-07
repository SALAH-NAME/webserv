/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/08/07 13:01:17 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(Socket sock, const ServerConfig& conf, int epfd, ClientInfos clientInfos) : _socket(sock)
											, _epfd(epfd)
											, _conf(conf)
											, _clientInfos(clientInfos)
											, _CGI_pipeFD(-1)
											, _lastTimeConnection(std::time(NULL))
											, _contentLength(0)
											, _uploadedBytes(0)
											, _responseHandler(new ResponseHandler(clientInfos, conf))
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
											, _isCgiRequired(CGI_IS_NOT_REQUIRED)
											, _isPipeReadable(PIPE_IS_NOT_READABLE)
											, _isPipeClosedByPeer(PIPE_IS_NOT_CLOSED)
											, _pipeReadComplete(READ_PIPE_NOT_COMPLETE)
											, _setTargetFile(false)
											, _responseSent(NOT_SENT)
{}

Client::Client(const Client& other) : _socket(other._socket)
									, _epfd(other._epfd)
									, _conf(other._conf)
									, _clientInfos(other._clientInfos)
									, _CGI_pipeFD(other._CGI_pipeFD)
									, _lastTimeConnection(other._lastTimeConnection)
									, _contentLength(other._contentLength)
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
									, _isCgiRequired(other._isCgiRequired)
									, _isPipeReadable(other._isPipeReadable)
									, _isPipeClosedByPeer(other._isPipeClosedByPeer)
									, _pipeReadComplete(other._pipeReadComplete)
									, _setTargetFile(other._setTargetFile)
									, _responseSent(other._responseSent)
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

int	Client::getCGI_pipeFD(void) {
	return _CGI_pipeFD;
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

bool	Client::getIsPipeClosedByPeer(void) {
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

void Client::appendToBodyPart(const std::string &bodyData)
{
	_requestBodyPart += bodyData;
}

void Client::setEvent(int _epfd, struct epoll_event &event)
{
	event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	epoll_ctl(_epfd, EPOLL_CTL_ADD, event.data.fd, &event);
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

void	Client::setIsPipeClosedByPeer(bool value) {
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
			std::cout << " ==> Sent Header Successfully <==\n";
		}
		else {
			_fullResponseFlag = FULL_RESPONSE_NOT_READY;
			_responseBodyFlag = RESPONSE_BODY_NOT_READY;
			_responseSent = SENT;
			std::cout << " ==> Sent Full Response Successfully <==\n";
		}
	}
}

void	Client::readFileBody(void) {
	std::fstream *targetFile = _responseHandler->GetTargetFilePtr();

	char buffer[BYTES_TO_SEND+1];
	targetFile->read(buffer, BYTES_TO_SEND);
	ssize_t _bytesReadFromFile = targetFile->gcount();

	buffer[_bytesReadFromFile] = 0;
	_responseHolder = std::string(buffer, _bytesReadFromFile);
	_isResponseBodySendable = SENDABLE;
}

void	Client::sendFileBody(void) {
	ssize_t sentBytes;
	
	if (_responseHolder.size()) {
		sentBytes = _socket.send(_responseHolder.c_str(), _responseHolder.size(), MSG_NOSIGNAL);
		if (sentBytes < 0)
			throwIfSocketError("send()");
		resetLastConnectionTime();
	}
	
	if (_responseHolder.size() < BYTES_TO_SEND) {
		_isResponseBodySendable = NOT_SENDABLE;
		_responseBodyFlag = RESPONSE_BODY_NOT_READY;
		_responseSent = SENT;
		std::cout << " ==>> Send response body Successfully \n";
		return ;
	}
	
	_isResponseBodySendable = NOT_SENDABLE;
}

void	Client::receiveRequestBody(void) {

	char buffer[BYTES_TO_READ+1];

	std::memset(buffer, 0, sizeof(buffer));

	if (!_contentLength) {
		_isRequestBodyWritable = NOT_WRITABLE;
		_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
		_fullResponseFlag = FULL_RESPONSE_READY;
		return ;
	}

	size_t	readBytes = _socket.recv(buffer, BYTES_TO_READ, MSG_DONTWAIT); // Enable NON_Blocking for recv()
	if (readBytes > 0 && readBytes <= BYTES_TO_READ) {
		resetLastConnectionTime();
		if (_uploadedBytes + readBytes >= _contentLength) {
			readBytes = _contentLength - _uploadedBytes;
		}
		buffer[readBytes] = 0;
		_requestBodyPart = std::string(buffer, readBytes);
		_isRequestBodyWritable = WRITABLE;
	}
	else if (readBytes == 0) {
		// waiting for timeout or clinet closed connection
	}
	else
		throwIfSocketError("recv()");	
}

void	Client::writeBodyToTargetFile(void) {
	std::fstream *targetFile = _responseHandler->GetTargetFilePtr();

	size_t BytesToWrite;

	if (_requestBodyPart.size() >= BYTES_TO_READ)
		BytesToWrite = BYTES_TO_READ;
	else
		BytesToWrite = _requestBodyPart.size();

	if ((_uploadedBytes + BytesToWrite) > _contentLength) {
		
		if (!_contentLength) {
			_isRequestBodyWritable = NOT_WRITABLE;
			_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
			_fullResponseFlag = FULL_RESPONSE_READY;
			return ;
		}
		BytesToWrite =_contentLength - _uploadedBytes;
	}
	
	targetFile->write(_requestBodyPart.c_str(), BytesToWrite);
	targetFile->flush();

	_uploadedBytes += BytesToWrite;
	_requestBodyPart = _requestBodyPart.substr(BytesToWrite);
	
	if (_requestBodyPart.size())
		_isRequestBodyWritable = WRITABLE;
	else
		_isRequestBodyWritable = NOT_WRITABLE;

	if (_uploadedBytes == _contentLength) {
		_isRequestBodyWritable = NOT_WRITABLE;
		_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
		_fullResponseFlag = FULL_RESPONSE_READY;
		return ;
	}
}

void	Client::closeAndDeregisterPipe(void) {
	std::cout << "============================================================\n";
	epoll_ctl(_epfd, EPOLL_CTL_DEL, _CGI_pipeFD, NULL);
	std::cout << "  ## Removed Pipe fd: " << _CGI_pipeFD << " from epoll  ## \n";
	close(_CGI_pipeFD);
	std::cout << "  ## closed Pipe fd: " << _CGI_pipeFD << "              ## \n";
	std::cout << "============================================================\n\n";
	
}

void	Client::CgiExceptionHandler(void) {
	if (_responseHandler->GetTargetFilePtr()) {
		// std::cout << "   ===>>> from target file \n";
		_responseHolder = _responseHandler->GetResponseHeader();
		_responseHeaderFlag = RESPONSE_HEADER_READY;
	}
	else {
		// std::cout << " ====>  no neeed \n";
		_responseHolder = _responseHandler->GetResponseHeader() + _responseHandler->GetResponseBody();
		_fullResponseFlag = FULL_RESPONSE_READY;
	}
}

void				Client::printClientStatus(void) {
	std::cout << "\n ------------------------------------------------------\n";
	std::cout << "  ## _CGI_pipeFD: " <<  _CGI_pipeFD << " ## \n";
	std::cout << "  ##  _requestHeaderPart size : " << _requestHeaderPart.size() << "  ## \n";
	std::cout << "  ##  _requestBodyPart size : " << _requestBodyPart.size() << "  ## \n";
	std::cout << "  ##  _responseHolder size : " << _responseHolder.size() << "  ## \n";
	// std::cout << "Response Holder {" << _responseHolder << "}\n";
	std::cout << "  ##  _pendingRequestDataHolder size : " << _pendingRequestDataHolder.size() << "  ## \n";
	std::cout << "  ##  _contentLength : " << _contentLength << "  ## \n";
	std::cout << "  ##  _uploadedBytes : " << _uploadedBytes << "  ## \n";
	std::cout << "  ##  _responseSent : " << _responseSent << "  ## \n";
	std::cout << "--------------------------------------------------------\n\n";

	printRequestAndResponse("Pending data", _pendingRequestDataHolder);

	// exit(0);
}

void	Client::resetAttributes(void) {
	_CGI_pipeFD =  -1;
	_lastTimeConnection =  std::time(NULL);
	_contentLength =  0;
	_uploadedBytes =  0;
	_httpRequest.reset();
	delete _responseHandler;
	_responseHandler = new ResponseHandler(_clientInfos, _conf);
	
	if (_requestDataPreloadedFlag == REQUEST_DATA_PRELOADED_ON)
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
	_isCgiRequired =  CGI_IS_NOT_REQUIRED;
	_isPipeReadable =  PIPE_IS_NOT_READABLE;
	_isPipeClosedByPeer =  PIPE_IS_NOT_CLOSED;
	_pipeReadComplete =  READ_PIPE_NOT_COMPLETE;
	_setTargetFile =  false;
	_responseSent = NOT_SENT;

	_requestHeaderPart.clear();
	_requestBodyPart.clear();
	_responseHolder.clear();


	std::cout << " ## RESETED ##\n";
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