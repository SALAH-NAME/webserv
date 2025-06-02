/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:40:16 by karim             #+#    #+#             */
/*   Updated: 2025/06/02 17:50:08 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int	Server::_id = 1;

void	Server::__init_attributes(void) {
	_domin = AF_INET;
	_type = SOCK_STREAM | SOCK_NONBLOCK;
	_protocol = 0;
	_2CRLF = "\r\n\r\n";
	_isKeepAlive = true;
	_ports.push_back(_serverConfig.getListen());
	_isSocketOwner = false;
	_timeOut = _serverConfig.getSessionTimeout();
}

Server::Server(const ServerConfig& serverConfig) : _serverConfig(serverConfig) {

	int socket_fd;
	__init_attributes();

	for (size_t i = 0; i < _ports.size(); i++) {
		
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
			_Address.sin_port = htons(_ports[i]);
			/*htons():  These functions shall convert 16-bit and 32-bit quantities between
			network byte order and host byte order.*/
			
			if (inet_pton(AF_INET, serverConfig.getHost().c_str(), &_Address.sin_addr) <= 0)
				throw "Invalid IP address";
		
			_bufferSize = sizeof(_buffer);
			memset(_buffer, 0, _bufferSize);
			
			if (bind(socket_fd, (sockaddr*)&_Address, sizeof(_Address)) < 0) {
				if (errno == EADDRINUSE) {
					_server_id = _id++;
					std::cout << "Server(" << _server_id << ") is listening on USED IP:PORT\n";
					std::cout << "==> this server is using used SOCKET\n";
					std::cout << "is socket owner => " << _isSocketOwner << "\n";
					close(socket_fd);
					return ;
				}
				else
					throw "bind failed";
			}
			else {
				/* _nMaxBacklog: the maximum length to which the queue
				of pending connections for sockfd may grow*/
				if (listen(socket_fd, _nMaxBacklog) < 0)
					throw "listen failed";
				_sockets_fds.push_back(socket_fd);
				_isSocketOwner = true;
				_server_id = _id++;
				std::cout << "Server(" << _server_id << ") {socket: " << socket_fd << "} is listening on => ";
				std::cout << serverConfig.getHost() << ":" << _ports[i] << "\n";
				std::cout << "is socket owner => " << _isSocketOwner << "\n";
			}
		}
		catch (const char* errorMssg) {
			perror(errorMssg);
			if (!(socket_fd < 0))
				close(socket_fd);
		}
		if (errno == EADDRINUSE)
			return ;
		
	}
	if (!_sockets_fds.size() || !_sockets_fds.size())
		throw "server failed";
}

Server::~Server(void) {
	// std::cout << "destructor called\n";
}

int		Server::get_id(void) {
	return _server_id;
}

void Server::set__epfd(int value) {
	_epfd = value;
}

std::vector<int>&		Server::get_sockets_fds() {
	return _sockets_fds;
}

std::vector<int>&	Server::get_clientsSockets(void) {
	return _clientsSockets;
}

bool	Server::verifyClientFD(int client_fd) {
	for (size_t i = 0; i < _clientsSockets.size(); i++) {
		if (client_fd == _clientsSockets[i])
			return true;
	}
	return false;
}

bool	Server::verifyServer_sockets_fds(int NewEvent_fd) {
	std::vector<int>::iterator it = std::find(_sockets_fds.begin(), _sockets_fds.end(), NewEvent_fd);
	return (it != _sockets_fds.end());
}


void	Server::closeConnection(int clientSocket) {
	epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSocket, NULL);
	_clientsSockets.erase(get_iterator(clientSocket, _clientsSockets));
	_clients.erase(clientSocket);
	close(clientSocket);
}

bool	Server::getIsSocketOwner(void) {
	return _isSocketOwner;
}

std::map<int, Client>& Server::getClients() {
	return _clients;
}

int	Server::getTimeOut(void) {
	return _timeOut;
}