/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:40:16 by karim             #+#    #+#             */
/*   Updated: 2025/06/30 16:21:23 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int	Server::_id = 1;

void	Server::initAttributes(void) {
	_domin = AF_INET;
	_type = SOCK_STREAM | SOCK_NONBLOCK;
	_protocol = 0;
	_ports.push_back(_serverConfig.getListen());
	_isSocketOwner = false;
	_timeOut = _serverConfig.getSessionTimeout();
}

Server::Server(const ServerConfig& serverConfig) : _serverConfig(serverConfig) {

	int socketFD;
	initAttributes();

	for (size_t i = 0; i < _ports.size(); i++) {
		
		try {
			if ((socketFD = socket(_domin, _type, _protocol)) < 0) {
				throw "socket failed: ";
			}	
			int reuse = 1;
			if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
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
			
			if (bind(socketFD, (sockaddr*)&_Address, sizeof(_Address)) < 0) {
				if (errno == EADDRINUSE) {
					_serverID = _id++;
					std::cout << "Server(" << _serverID << ") is listening on USED IP:PORT\n";
					std::cout << "==> this server is using used SOCKET\n";
					std::cout << "is socket owner => " << _isSocketOwner << "\n";
					close(socketFD);
					return ;
				}
				else
					throw "bind failed";
			}
			else {
				/* _nMaxBacklog: the maximum length to which the queue
				of pending connections for sockfd may grow*/
				if (listen(socketFD, _nMaxBacklog) < 0)
					throw "listen failed";
				_socketsFDs.push_back(socketFD);
				_isSocketOwner = true;
				_serverID = _id++;
				std::cout << "Server(" << _serverID << ") {socket: " << socketFD << "} is listening on => ";
				std::cout << serverConfig.getHost() << ":" << _ports[i] << "\n";
				std::cout << "is socket owner => " << _isSocketOwner << "\n";
			}
		}
		catch (const char* errorMssg) {
			perror(errorMssg);
			if (!(socketFD < 0))
				close(socketFD);
		}
		if (errno == EADDRINUSE)
			return ;
		
	}
	if (!_socketsFDs.size() || !_socketsFDs.size())
		throw "server failed";
}

Server::~Server(void) {
	// std::cout << "destructor called\n";
}

int		Server::getID(void) {
	return _serverID;
}

void Server::setEPFD(int value) {
	_epfd = value;
}

std::vector<int>&		Server::getSocketsFDs() {
	return _socketsFDs;
}

std::vector<int>&	Server::getClientsSockets(void) {
	return _clientsSockets;
}

bool	Server::verifyClientFD(int client_fd) {
	for (size_t i = 0; i < _clientsSockets.size(); i++) {
		if (client_fd == _clientsSockets[i])
			return true;
	}
	return false;
}

bool	Server::verifyServerSocketsFDs(int NewEvent_fd) {
	std::vector<int>::iterator it = std::find(_socketsFDs.begin(), _socketsFDs.end(), NewEvent_fd);
	return (it != _socketsFDs.end());
}


void	Server::closeConnection(int clientSocket) {
	epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSocket, NULL);
	_clientsSockets.erase(getIterator(clientSocket, _clientsSockets));
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
