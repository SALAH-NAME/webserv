/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/07/03 14:30:14 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "HttpRequest.hpp"

Client::Client(void) {}

Client::Client(int fd, int socketFD) : _socketFD(fd), _serverSocketFD(socketFD),
											_readBytes(0), _timeOut(std::time(NULL)),
											_incomingDataDetected(INCOMING_DATA_OFF),
											_responseInFlight(false), _sentBytes(0),
											_isKeepAlive(true),
											_availableResponseBytes(RESPONSESIZE) {}

int			Client::getFD() {
	return _socketFD;
}
											
size_t	Client::getReadBytes(void) {
	return _readBytes;
}

std::string	Client::getRequest(void) {
	return _requestHolder;
}

int		Client::getServerSocketFD(void) {
	return _serverSocketFD;
}

std::string &Client::getResponse(void) {
	return _responseHolder;
}


time_t		Client::getLastConnectionTime(void){
	return _timeOut;
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

void	Client::setReadBytes(size_t bytes) {
	_readBytes += bytes;
}

void	Client::setResponseInFlight(bool value) {
	_responseInFlight = value;
}

bool	Client::getResponseInFlight(void) {
	return _responseInFlight;
}

void		Client::appendToRequest(const std::string& requestData) {
	_requestHolder += requestData;
}

void	Client::setEvent(int _epfd, struct epoll_event& event) {
	event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	epoll_ctl(_epfd, EPOLL_CTL_ADD, event.data.fd, &event);
}

void    Client::setServerSocketFD(int s_fd) {
	_serverSocketFD = s_fd;
}

void		 Client::setResponse(std::string response) {
	_responseHolder += response;	
}

void	Client::setIncomingDataFlag(bool flag) {
	_incomingDataDetected = flag;
}

void	Client::resetLastConnectionTime(void){
	_timeOut = std::time(NULL);
}

void	Client::setSentBytes(size_t bytes) {
	_sentBytes += bytes;
	_availableResponseBytes -= bytes;
}

void	Client::resetSendBytes(void) {
	_sentBytes = 0;
	_availableResponseBytes = RESPONSESIZE;
}

void	Client::setIncomingDataDetected(int mode) {
	_incomingDataDetected = mode;
}

void	Client::clearRequestHolder(void) {
	_requestHolder.clear();
}

bool		Client::parseRequest() {
	return _requestInfos.parse(_requestHolder);
}

void	Client::prinfRequestinfos(void) {
	_requestInfos.printInfos();
}
