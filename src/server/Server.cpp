/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:40:16 by karim             #+#    #+#             */
/*   Updated: 2025/07/28 17:27:26 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::initAttributes(int id) {
	_domin = AF_INET;
	_type = SOCK_STREAM | SOCK_NONBLOCK;
	_protocol = 0;
	_ports = _serverConfig.getListens();
	_timeOut = _serverConfig.getConnectionTimeout();
	_id = id;
}

Server::Server(const ServerConfig& serverConfig, size_t id) : _serverConfig(serverConfig), _id(id), _transferSocket() {
	initAttributes(id);

	for (size_t i = 0; i < _ports.size(); i++) {
		
		try {
			_listeningSockets.push_back(Socket());
		
			_listeningSockets[_listeningSockets.size() - 1].create();
			
			int reuse = 1;
			_listeningSockets[_listeningSockets.size() - 1].setsockopt(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
			// fixed this problem ==>  the OS keeps the port in a "cool-down" period (TIME_WAIT)
			// ==> It’s mainly for quick restart development or for binding during graceful restarts.
			
			std::memset(&_Address, 0, sizeof(_Address));
			_Address.sin_family = _domin;
			_Address.sin_port = htons(_ports[i]);
			
			if (inet_pton(AF_INET, serverConfig.getHost().c_str(), &_Address.sin_addr) <= 0)
				throw std::runtime_error(std::string("Invalid IP address: ") + strerror(errno));

			struct sockaddr addr;
			std::memcpy(&addr, &_Address, sizeof(_Address));

			_listeningSockets[_listeningSockets.size() - 1].bind(&addr, sizeof(_Address));
			_listeningSockets[_listeningSockets.size() - 1].listen();
			std::cout << "Server(" << _id << ") {socket: " << _listeningSockets[_listeningSockets.size() - 1].getFd() << "} is listening on => ";
			std::cout << serverConfig.getHost() << ":" << _ports[i] << "\n";
		}
		catch (const std::runtime_error& e) {
			_listeningSockets.pop_back();
			perror(e.what());
		}
	}
	if (!_listeningSockets.size())
		throw "server failed";
}

Server::~Server(void) {}

int		Server::getID(void) {
	return _id;
}

std::vector<int>	Server::getMarkedForEraseClients() {
	return _markedForEraseClients;
}

void Server::setEPFD(int value) {
	_epfd = value;
}

std::vector<Socket>&		Server::getListeningSockets() {
	return _listeningSockets;
}

bool	Server::verifyClientsFD(int client_fd) {
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if (client_fd == it->first)
			return true ;
	}
	return false;
}

bool	Server::verifyServerSocketsFDs(int NewEvent_fd) {
	for (size_t i = 0; i < _listeningSockets.size(); i++) {
		if (NewEvent_fd == _listeningSockets[i].getFd())
			return true;
	}
	return false;
}


void	Server::closeConnection(int clientSocket) {
	epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSocket, NULL);
	_markedForEraseClients.push_back(clientSocket);
}

void	Server::eraseMarked() {
	for (size_t i = 0; i < _markedForEraseClients.size(); i++) {
		close(_markedForEraseClients[i]);
		std::cout << "close connection: " << _markedForEraseClients[i] << "\n";
		_clients.erase(_markedForEraseClients[i]);
	}
	_markedForEraseClients.clear();
	// if (_clients.size())
		// std::cout << "client size: " << _clients.size() << "\n";
}

std::map<int, Client>& Server::getClients() {
	return _clients;
}

int	Server::getTimeOut(void) {
	return _timeOut;
}

Socket&	Server::getTransferSocket(void) {
	return _transferSocket;
}
