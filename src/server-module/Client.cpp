/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/06/25 19:44:07 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "HttpRequest.hpp"

Client::Client(void) {}

Client::Client(int fd, int socket_fd) : _socket_fd(fd), _serverSocket_fd(socket_fd),
											_readBytes(0), _timeOut(std::time(NULL)) {}

void	Client::setReadBytes(size_t bytes) {
	_readBytes += bytes;
}

size_t	Client::getReadBytes(void) {
	return _readBytes;
}
											
int			Client::getFD() {
	return _socket_fd;
}

void		Client::setRequest(std::string requestData) {
	_requestHolder += requestData;
}

std::string	Client::getRequest(void) {
	return _requestHolder;
}

void    Client::set_serverSocketFD(int s_fd) {
	_serverSocket_fd = s_fd;
}

int		Client::get_serverSocketFD(void) {
	return _serverSocket_fd;
}

void		 Client::setResponse(std::string response) {
	_responseHolder += response;	
}

std::string &Client::getResponse(void) {
	return _responseHolder;
}

void	Client::clearRequestHolder(void) {
	_requestHolder.clear();
}

void	Client::resetLastConnectionTime(void){
	_timeOut = std::time(NULL);
}

time_t		Client::getLastConnectionTime(void){
	return _timeOut;
}

bool		Client::parseRequest() {
	return _requestInfos.parse(_requestHolder);
}
