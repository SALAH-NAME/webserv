/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:40:16 by karim             #+#    #+#             */
/*   Updated: 2025/05/13 09:29:25 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::map<std::string, int>& infos) {
	port = infos["port"];
	_domin = infos["domain"];
	_type = infos["type"];
	_protocol = infos["protocol"];

	if ((socket_fd = socket(_domin, _type, _protocol)) < 0)
		throw "socket failed: ";

	int reuse = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	_Address.sin_family = _domin;
	_Address.sin_addr.s_addr = _protocol;
	_Address.sin_port = htons(port);
	/*htons():  These functions shall convert 16-bit and 32-bit quantities between
				network byte order and host byte order.*/

	bufferSize = sizeof(buffer);
	memset(buffer, 0, bufferSize);

	if (bind(socket_fd, (sockaddr*)&_Address, sizeof(_Address)) < 0) {
		close(socket_fd);	
		throw "bind failed";
	}

	/* _nMaxBacklog: the maximum length to which the queue
		of pending connections for sockfd may grow*/
	if (listen(socket_fd, _nMaxBacklog) < 0)
		throw "listen failed";

	std::cout << "Server {" << socket_fd << "} is listening on port " << port << "...\n";

}

Server::~Server(void) {
	// std::cout << "destructor called\n";
}

void Server::set_epfd(int value) {
	epfd = value;
}

struct epoll_event & Server::getTarget() {
	return targetInfos;
}

int		Server::getSocket_fd() {
	return socket_fd;
}

void	Server::set_nfds(int value) {
	nfds = value;	
}

std::map<int, std::time_t>&	Server::get_clientsSockets(void) {
	return clientsSockets;
}

std::vector<int>&	Server::get_responseWaitQueue(void) {
	return responseWaitQueue;
}

bool	Server::verifyClientFD(int client_fd) {
	std::map<int, std::time_t>::iterator it = clientsSockets.find(client_fd);
	return (it != clientsSockets.end());
}
