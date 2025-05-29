/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:40:16 by karim             #+#    #+#             */
/*   Updated: 2025/05/28 17:00:16 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int	Server::_id = 1;

void	Server::__init_attributes(const ServerConfig& _serverInfo) {
	_domin = AF_INET;
	_type = SOCK_STREAM | SOCK_NONBLOCK;
	_protocol = 0;
	_host = _serverInfo.getHost();
	_timeout = _serverInfo.getSessionTimeout();
	_2CRLF = "\r\n\r\n";
	_isKeepAlive = true;
	debug = 0;

	ports.push_back(_serverInfo.getListen());
}

Server::Server(const ServerConfig& _serverInfo) {

	int socket_fd;
	__init_attributes(_serverInfo);

	for (size_t i = 0; i < ports.size(); i++) {
		
		try {
			if ((socket_fd = socket(_domin, _type, _protocol)) < 0) {
				throw "socket failed: ";
			}	
				int reuse = 1;
				if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
					throw "setsockopt(SO_REUSEADDR) failed";
				// fixed this problem ==>  the OS keeps the port in a "cool-down" period (TIME_WAIT)
				// ==> It’s mainly for quick restart development or for binding during graceful restarts.
				
				memset(&_Address, 0, sizeof(_Address));
				_Address.sin_family = _domin;
				_Address.sin_port = htons(ports[i]);
				/*htons():  These functions shall convert 16-bit and 32-bit quantities between
				network byte order and host byte order.*/
				
				if (inet_pton(AF_INET, _host.c_str(), &_Address.sin_addr) <= 0)
					throw "Invalid IP address";
			
				bufferSize = sizeof(buffer);
				memset(buffer, 0, bufferSize);
				
				if (bind(socket_fd, (sockaddr*)&_Address, sizeof(_Address)) < 0) {
					throw "bind failed";
				}
				
				/* _nMaxBacklog: the maximum length to which the queue
				of pending connections for sockfd may grow*/
				if (listen(socket_fd, _nMaxBacklog) < 0)
					throw "listen failed";
				
			sockets_fds.push_back(socket_fd);
			server_id = _id++;

			
			std::cout << "Server(" << server_id << ") {socket: " << socket_fd << "} is listening on => ";
			std::cout << _host << ":" << ports[i] << "\n";
		}
		catch (const char* errorMssg) {
			perror(errorMssg);
			if (!(socket_fd < 0))
				close(socket_fd);
		}
		
	}
}

Server::~Server(void) {
	// std::cout << "destructor called\n";
}

unsigned int	Server::getTimeout(void) {
	return _timeout;
}

int		Server::get_id(void) {
	return server_id;
}

void Server::set_epfd(int value) {
	epfd = value;
}

struct epoll_event & Server::getTarget() {
	return targetInfos;
}

std::vector<int>&		Server::getSockets_fds() {
	return sockets_fds;
}

void	Server::set_nfds(int value) {
	nfds = value;	
}

std::vector<int>&	Server::get_clientsSockets(void) {
	return clientsSockets;
}

// std::vector<int>&	Server::get_responseWaitQueue(void) {
// 	return responseWaitQueue;
// }

bool	Server::verifyClientFD(int client_fd) {
	// std::map<int, std::time_t>::iterator it = clientsSockets.find(client_fd);
	// return (it != clientsSockets.end());

	for (size_t i = 0; i < clientsSockets.size(); i++) {
		if (client_fd == clientsSockets[i])
			return true;
	}
	return false;
}

bool	Server::verifyServerSockets_fds(int NewEvent_fd) {
	std::vector<int>::iterator it = std::find(sockets_fds.begin(), sockets_fds.end(), NewEvent_fd);
	return (it != sockets_fds.end());
}