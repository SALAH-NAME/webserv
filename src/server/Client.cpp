/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/07/23 11:56:46 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "HttpRequest.hpp"

Client::Client(Socket sock, int serverFD, const ServerConfig& conf) : _socket(sock), _serverSocketFD(serverFD),
											_readBytes(0), _lastTimeConnection(std::time(NULL)),
											_incomingDataDetected(INCOMING_DATA_OFF),
											_responseInFlight(false), _sentBytes(0),
											_isKeepAlive(true),
											_availableResponseBytes(0),
											_generateInProcess(GENERATE_RESPONSE_OFF)
											, _responseHandler(new ResponseHandler("0.0.0.0", conf))
											, _shouldTransferBody(TRANSFER_BODY_OFF)
											, _bodySize(0), _contentLength(0)
											, _bodyDataPreloaded(BODY_DATA_PRELOADED_OFF)
{}

Client::~Client() {}

Socket &Client::getSocket()
{
	return _socket;
}

size_t Client::getReadBytes(void)
{
	return _readBytes;
}

int Client::getServerSocketFD(void)
{
	return _serverSocketFD;
}

time_t Client::getLastConnectionTime(void)
{
	return _lastTimeConnection;
}

bool	Client::getIncomingDataDetectedFlag(void) {
	return _incomingDataDetected;
}

bool Client::getIsKeepAlive(void)
{
	return _isKeepAlive;
}

size_t Client::getSentBytes(void)
{
	return _sentBytes;
}

int Client::getBytesToSendNow(void)
{
	if (_availableResponseBytes >= BYTES_TO_SEND)
		return BYTES_TO_SEND;
	return _availableResponseBytes;
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

size_t Client::getAvailableResponseBytes(void)
{
	return _availableResponseBytes;
}

bool	Client::getShouldTransferBody(void) {
	return _shouldTransferBody;
}

void	Client::setReadBytes(size_t bytes) {
	_readBytes += bytes;
}

void Client::setResponseInFlight(bool value)
{
	_responseInFlight = value;
}

bool Client::getResponseInFlight(void)
{
	return _responseInFlight;
}

size_t Client::getResponseSize(void)
{
	return _responseSize;
}

std::string&	Client::getResponseHolder(void) {
	return _responseHolder;
}

size_t	Client::getBodySize(void) {
	return _bodySize;
}

bool	Client::getBodyDataPreloaded(void) {
	return _bodyDataPreloaded;
}

size_t	Client::getContentLength(void) {
	return _contentLength;
}

std::string	Client::getLastReceivedHeaderData(void) {
	return _lastReceivedHeaderData;
}

void		Client::appendToHeaderPart(const std::string& headerData) {
	_requestHeaderPart += headerData;
	_lastReceivedHeaderData.clear();
	_lastReceivedHeaderData = headerData;
}

void Client::appendToBodyPart(const std::string &bodyData)
{
	_requestBodyPart += bodyData;
	_bodySize += bodyData.size();
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

void Client::setSentBytes(size_t bytes)
{
	_sentBytes += bytes;
	_availableResponseBytes -= bytes;
}

void Client::resetSendBytes(void)
{
	_sentBytes = 0;
	_availableResponseBytes = 0;
}

void	Client::setIncomingDataDetectedFlag(int mode) {
	_incomingDataDetected = mode;
}

void Client::setGenerateResponseInProcess(bool value)
{
	_generateInProcess = value;
}

void Client::setResponseSize(size_t size)
{
	_responseSize = size;
}

void Client::setAvailableResponseBytes(size_t value)
{
	_availableResponseBytes = value;
}

void	Client::setBodyDataPreloaded(bool value) {
	_bodyDataPreloaded = value;
}

void	Client::setRequestBodyPart(std::string bodyData) {
	_requestBodyPart = bodyData;
	_bodySize = bodyData.size();
}

void Client::resetBodySize(void) {
	_bodySize = 0;
}

void	Client::setContentLength(int length) {
	_contentLength = length;
}

void	Client::resetContentLength(void) {
	_contentLength = 0;
}

void	Client::setShouldTransferBody(bool value) {
	_shouldTransferBody = value;
}

void	Client::setHeaderPart(std::string HeaderData) {
	_requestHeaderPart = HeaderData;
}

void	Client::clearRequestHolder(void) {
	_requestHeaderPart.clear();
	_requestBodyPart.clear();
	_httpRequest.reset(); // Reset the incremental parser state
}

bool Client::parseRequest()
{
	return _httpRequest.parse(_requestHeaderPart);
}

void Client::prinfRequestinfos(void)
{
	_httpRequest.printInfos();
}

void	Client::trimBufferedBodyToContentLength(void) {
	if (_requestBodyPart.size() < _contentLength) {
		_bodyDataPreloaded = BODY_DATA_PRELOADED_OFF;
	}
	else if (_requestBodyPart.size() == _contentLength) {
		_shouldTransferBody = TRANSFER_BODY_OFF;
		_responseInFlight = true;
	}
	else {

		_requestBodyPart = _requestBodyPart.substr(0, _contentLength);
		_bodyDataPreloaded = BODY_DATA_PRELOADED_OFF;
		if (_requestBodyPart.size() == _contentLength) {
			_shouldTransferBody = TRANSFER_BODY_OFF;
			_responseInFlight = true;
		}
	}
	writeToTargetFile(_requestBodyPart);
}

void	Client::writeToTargetFile(const std::string& data) {
	std::fstream *targetFile = _responseHandler->GetTargetFilePtr();
	targetFile->write(data.c_str(), data.size());
	targetFile->flush();
}