/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/07/19 18:35:00 by karim            ###   ########.fr       */
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
											, _responseHandler(new ResponseHandler(conf))
{}

Client::~Client() {}

Socket&			Client::getSocket() {
	return _socket;
}
											
size_t	Client::getReadBytes(void) {
	return _readBytes;
}

int		Client::getServerSocketFD(void) {
	return _serverSocketFD;
}

time_t		Client::getLastConnectionTime(void){
	return _lastTimeConnection;
}

bool	Client::getIncomingDataDetected(void) {
	return _incomingDataDetected;
}

bool	Client::getIsKeepAlive(void) {
	return _isKeepAlive;
}

size_t	Client::getSentBytes(void) {
	return _sentBytes;
}

int	Client::getBytesToSendNow(void) {
	if (_availableResponseBytes >= BYTES_TO_SEND)
		return BYTES_TO_SEND;
	return _availableResponseBytes;
}

bool	Client::getGenerateInProcess(void) {
	return _generateInProcess;
}

HttpRequest&	Client::getHttpRequest(void) {
	return _httpRequest;
}

std::string&	Client::getHeaderPart(void) {
	return	_requestHeaderPart;
}

std::string&	Client::getBodyPart(void) {
	return	_requestBodyPart;
}

size_t	Client::getAvailableResponseBytes(void) {
	return _availableResponseBytes;
}

void	Client::setReadBytes(size_t bytes) {
	_readBytes += bytes;
}

void	Client::setResponseInFlight(bool value) {
	_responseInFlight = value;
}

bool	Client::getResponseInFlight(void) {
	return _responseInFlight;
}

size_t	Client::getResponseSize(void) {
	return _responseSize;
}

void		Client::appendToHeaderPart(const std::string& headerData) {
	_requestHeaderPart += headerData;
}

void		Client::appendToBodyPart(const std::string& bodyData) {
	_requestBodyPart += bodyData;
}

void	Client::setEvent(int _epfd, struct epoll_event& event) {
	event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	epoll_ctl(_epfd, EPOLL_CTL_ADD, event.data.fd, &event);
}

void    Client::setServerSocketFD(int s_fd) {
	_serverSocketFD = s_fd;
}

void	Client::setIncomingDataFlag(bool flag) {
	_incomingDataDetected = flag;
}

void	Client::resetLastConnectionTime(void){
	_lastTimeConnection = std::time(NULL);
}

void	Client::setSentBytes(size_t bytes) {
	_sentBytes += bytes;
	_availableResponseBytes -= bytes;
}

void	Client::resetSendBytes(void) {
	_sentBytes = 0;
	_availableResponseBytes = 0;
}

void	Client::setIncomingDataDetected(int mode) {
	_incomingDataDetected = mode;
}

void	Client::setGenerateResponseInProcess(bool value) {
	_generateInProcess = value;
}

void	Client::setResponseSize(size_t size) {
	_responseSize = size;
}

void	Client::setAvailableResponseBytes(size_t value) {
	_availableResponseBytes = value;
}

void	Client::clearRequestHolder(void) {
	_requestHeaderPart.clear();
}

bool		Client::parseRequest() {
	return _httpRequest.parse(_requestHeaderPart);
}

void	Client::prinfRequestinfos(void) {
	_httpRequest.printInfos();
}
