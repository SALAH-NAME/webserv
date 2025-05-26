/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:42:11 by karim             #+#    #+#             */
/*   Updated: 2025/05/17 16:27:28 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <cstdlib>
#include <iostream>

Request::Request(void) {}

Request::Request(int fd, int socket_fd) : socket_fd(fd), serverSocket_fd(socket_fd),
											readBytes(0) {}

void	Request::setReadBytes(size_t bytes) {
	readBytes += bytes;
}

size_t	Request::getReadBytes(void) {
	return readBytes;
}
											
int			Request::getFD() {
	return socket_fd;
}

void		Request::setRequest(std::string requestData) {
	requestHolder += requestData;
}

std::string	Request::getRequest(void) {
	return requestHolder;
}

void    Request::set_serverSocketFD(int s_fd) {
	serverSocket_fd = s_fd;
}


int		Request::get_serverSocketFD(void) {
	return serverSocket_fd;
}