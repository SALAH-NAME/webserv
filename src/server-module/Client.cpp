/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/06/30 12:38:07 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "HttpRequest.hpp"

Client::Client(void) {}

Client::Client(int fd, int socket_fd) : _socket_fd(fd), _serverSocket_fd(socket_fd),
											_readBytes(0), _timeOut(std::time(NULL)),
											_incomingDataDetected(INCOMING_DATA_OFF),
											_isKeepAlive(true), _sentBytes(0),
											_availableResponseBytes(RESPONSESIZE) {}

int			Client::getFD() {
	return _socket_fd;
}
											
size_t	Client::getReadBytes(void) {
	return _readBytes;
}

std::string	Client::getRequest(void) {
	return _requestHolder;
}

int		Client::get_serverSocketFD(void) {
	return _serverSocket_fd;
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

struct epoll_event&	Client::getEvent() {
	return _event;
}

bool	Client::getIsKeepAlive(void) {
	return _isKeepAlive;
}

int	Client::getSentBytes(void) {
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

void	Client::setEvent(struct epoll_event& event) {
	memcpy(&_event, &event, sizeof(event));
}

void    Client::set_serverSocketFD(int s_fd) {
	_serverSocket_fd = s_fd;
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

void	Client::setEventStatus(int epfd, struct epoll_event& event) {
	_event.events = EPOLLIN | EPOLLOUT;  // enable write temporarily
	epoll_ctl(epfd, EPOLL_CTL_MOD, _socket_fd, &_event);
}

void	Client::setSentBytes(int bytes) {
	_sentBytes += bytes;
	_availableResponseBytes -= _sentBytes;
}

void	Client::resetSendBytes(void) {
	_sentBytes = 0;
	_availableResponseBytes = RESPONSESIZE;
}

void	Client::clearRequestHolder(void) {
	_requestHolder.clear();
}

bool		Client::parseRequest() {
	return _requestInfos.parse(_requestHolder);
}
