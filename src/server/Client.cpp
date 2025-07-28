/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/07/28 18:14:30 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "HttpRequest.hpp"


Client::Client(Socket sock, int serverFD, const ServerConfig& conf) : _socket(sock), _serverSocketFD(serverFD),
											_lastTimeConnection(std::time(NULL))
											, _incomingHeaderDataDetected(INCOMING_DATA_HEADER_OFF)
											, _responseHeaderFlag(RESPONSE_HEADER_NOT_READY)
											, _responseBodyFlag(RESPONSE_BODY_NOT_READY)
											, _fullResponseFlag(FULL_RESPONSE_NOT_READY)
											, _uploadedBytes(0)
											// , _savedBodyBytes(0)
											, _sentBytes(0)
											, _isKeepAlive(true)
											, _generateInProcess(GENERATE_RESPONSE_OFF)
											, _responseHandler(new ResponseHandler("0.0.0.0", conf))
											, _incomingBodyDataDetectedFlag(INCOMING_BODY_DATA_OFF)
											// , _bodySize(0)
											, _contentLength(0)
											, _isResponseBodySendable(NOT_SENDABLE)
											, _isRequestBodyWritable(NOT_WRITABLE)
											// , _bytesReadFromFile(0)
											, _bodyDataPreloaded(BODY_DATA_PRELOADED_OFF)
{}

Client::~Client() {}

Socket &Client::getSocket()
{
	return _socket;
}

int Client::getServerSocketFD(void)
{
	return _serverSocketFD;
}

time_t Client::getLastConnectionTime(void)
{
	return _lastTimeConnection;
}

bool	Client::getIncomingHeaderDataDetectedFlag(void) {
	return _incomingHeaderDataDetected;
}

std::string&	Client::getRequestBodyPart(void) {
	return _requestBodyPart;
}

bool Client::getIsKeepAlive(void)
{
	return _isKeepAlive;
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

size_t Client::getResponseSize(void)
{
	return _responseSize;
}

size_t	Client::getUploadedBytes(void) {
	return _uploadedBytes;
}

bool	Client::getBodyDataPreloaded(void) {
	return _bodyDataPreloaded;
}

size_t	Client::getContentLength(void) {
	return _contentLength;
}

std::string&		Client::getResponseHolder(void) {
	return _responseHolder;
}

bool	Client::getIsResponseBodySendable(void) {
	return _isResponseBodySendable;
}

// size_t	Client::getSavedBytes(void) {
// 	return _savedBodyBytes;
// }

void		Client::appendToHeaderPart(const std::string& headerData) {
	_requestHeaderPart += headerData;
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

void Client::setServerSocketFD(int s_fd)
{
	_serverSocketFD = s_fd;
}

void	Client::resetLastConnectionTime(void){
	_lastTimeConnection = std::time(NULL);
}

void	Client::setIncomingHeaderDataDetectedFlag(int mode) {
	_incomingHeaderDataDetected = mode;
}

void Client::setGenerateResponseInProcess(bool value)
{
	_generateInProcess = value;
}

void Client::setResponseSize(size_t size)
{
	_responseSize = size;
}

void	Client::setBodyDataPreloaded(bool value) {
	_bodyDataPreloaded = value;
}

void	Client::setRequestBodyPart(std::string bodyData) {
	_requestBodyPart = bodyData;
	// _bodySize = bodyData.size();
}

void	Client::setUploadedBytes(size_t bytes) {
	_uploadedBytes = bytes;
}

// void	Client::setSavedBodyBytes(size_t savedBytes) {
// 	_savedBodyBytes = savedBytes;
// }

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

void	Client::setIsRequestBodyWritable(bool value) {
	_isRequestBodyWritable = value;
}

// void	Client::clearRequestHolder(void) {
// 	_requestHeaderPart.clear();
// 	_requestBodyPart.clear();
// 	_httpRequest.reset(); // Reset the incremental parser state
// }

bool Client::parseRequest()
{
	return _httpRequest.parse(_requestHeaderPart);
}

void Client::prinfRequestinfos(void)
{
	_httpRequest.printInfos();
}

bool	Client::updateHeaderStateAfterSend(size_t sentBytes) {
	_responseHolder = _responseHolder.substr(sentBytes);
	if (!_responseHolder.size()) {
		if (_responseHeaderFlag == RESPONSE_HEADER_READY) {
			_responseHeaderFlag = RESPONSE_HEADER_NOT_READY;
			_responseBodyFlag = RESPONSE_BODY_READY;
		}
		else {
			_fullResponseFlag = FULL_RESPONSE_NOT_READY;
			_responseBodyFlag = RESPONSE_BODY_NOT_READY;
			return true;
		}
	}
	return false;
}

bool	Client::readFileBody(void) {

	std::fstream *targetFile = _responseHandler->GetTargetFilePtr();

	char buffer[BYTES_TO_SEND+1];
	targetFile->read(buffer, BYTES_TO_SEND);
	ssize_t _bytesReadFromFile = targetFile->gcount();
	if (_bytesReadFromFile == 0) {
		_responseBodyFlag = RESPONSE_BODY_NOT_READY;
		_isResponseBodySendable = NOT_SENDABLE;
		return true;
	}

	buffer[_bytesReadFromFile] = 0;
	_responseHolder = std::string(buffer, _bytesReadFromFile);

	_isResponseBodySendable = SENDABLE;
	return false;

}

bool	Client::sendFileBody(void) {

	ssize_t sentBytes = _socket.send(_responseHolder.c_str(), _responseHolder.size());
	if (sentBytes < 0)
		throwIfSocketError("send()");
	
	resetLastConnectionTime();
	if (_responseHolder.size() < BYTES_TO_SEND) {
		_isResponseBodySendable = NOT_SENDABLE;
		_responseBodyFlag = RESPONSE_BODY_NOT_READY;
		return true;
	}

	_isResponseBodySendable = NOT_SENDABLE;
	return false;
}

void	Client::receiveRequestBody(void) {

	char buffer[BYTES_TO_READ+1];

	std::memset(buffer, 0, sizeof(buffer));

	size_t	readBytes = _socket.recv(buffer, BYTES_TO_READ);
	if (readBytes > 0) {
		resetLastConnectionTime();
		if (_uploadedBytes + readBytes >= _contentLength) {
			// std::cout << " **** the body size is bigger than the content length: " << _contentLength << " ****\n\n";
			readBytes = _contentLength - _uploadedBytes;
		}
		buffer[readBytes] = 0;
		_requestBodyPart = std::string(buffer, readBytes);
		_isRequestBodyWritable = WRITABLE;
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

	targetFile->write(_requestBodyPart.c_str(), BytesToWrite);
	if (!*targetFile) {
		std::cerr << "\n ##########  Write failed #######\n\n";
	}
	
	targetFile->flush();

	_uploadedBytes += BytesToWrite;
	_requestBodyPart = _requestBodyPart.substr(BytesToWrite);
	// std::cout << "UPLOADED BYTES ==> " << _uploadedBytes << "\n";
	
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