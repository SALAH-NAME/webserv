/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/05/28 16:55:36 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>

Client::Client(void) {}

Client::Client(int fd, int socket_fd) : socket_fd(fd), serverSocket_fd(socket_fd),
											readBytes(0), outStatus(false) {}

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

// void	Client::addNewEvent(struct epoll_event newEvent) {
// 	events.push_back(newEvent);
// }

// std::vector<struct epoll_event>& Client::getEvents(void) {
// 	return events;
// }

void	Client::setOutStatus(bool status) {
	outStatus = status;
}

bool	Client::getOutStatus(void) {
	return outStatus;
}

void		 Client::setResponse(std::string response) {
	responseHolder += response;	
}

std::string &Client::getResponse(void) {
	return responseHolder;
}