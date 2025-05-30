/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/05/30 12:43:48 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>
#include <ctime>

Client::Client(void) {}

Client::Client(int fd, int socket_fd) : socket_fd(fd), serverSocket_fd(socket_fd),
											readBytes(0), outStatus(0), timeOut(std::time(NULL)) {}

void	Client::setReadBytes(size_t bytes) {
	readBytes += bytes;
}

size_t	Client::getReadBytes(void) {
	return readBytes;
}
											
int			Client::getFD() {
	return socket_fd;
}

void		Client::setRequest(std::string requestData) {
	requestHolder += requestData;
}

std::string	Client::getRequest(void) {
	return requestHolder;
}

void    Client::set_serverSocketFD(int s_fd) {
	serverSocket_fd = s_fd;
}

int		Client::get_serverSocketFD(void) {
	return serverSocket_fd;
}

void	Client::setOutStatus(int status) {
	outStatus = status;
}

int	Client::getOutStatus(void) {
	return outStatus;
}

void		 Client::setResponse(std::string response) {
	responseHolder += response;	
}

std::string &Client::getResponse(void) {
	return responseHolder;
}

void	Client::clearRequestHolder(void) {
	requestHolder.clear();
}

void	Client::resetLastConnectionTime(void){
	timeOut = std::time(NULL);
}

time_t		Client::getLastConnectionTime(void){
	return timeOut;
}