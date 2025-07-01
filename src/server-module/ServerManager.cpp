/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 10:25:03 by karim             #+#    #+#             */
/*   Updated: 2025/06/30 16:15:50 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"

void	ServerManager::checkTimeOut(void) {

	for (size_t x = 0; x < _servers.size(); x++) {
		
		std::map<int, Client>&	clients = _servers[x].getClients();
		std::vector<int>&		clientsSockets = _servers[x].getClientsSockets();
		
		for (size_t i = 0; i < clients.size(); i++) {
			if (std::time(NULL) - clients[clientsSockets[i]].getLastConnectionTime() > _servers[x].getTimeOut()) {
				_servers[x].closeConnection(clientsSockets[i]);
				i--;
			}
		}
	}
}

void	ServerManager::setUpServers(void) {
	for (size_t i = 0; i < _serversConfig.size(); i++) {
		try {
			_servers.push_back(Server(_serversConfig[i]));
		}
		catch (const char *errorMssg) {
			perror(errorMssg);
		}
	}
	if (!_servers.size())
		throw "No server is available";
}

void    ServerManager::setEpoll(void) {
	std::cout << "----------------- Set Epoll ----------------------\n";
	_epfd = epoll_create1(0);
	if (_epfd == -1)
		throw ("epoll create1 failed");
	std::cout << "an epoll instance for the servers sockets created(" << _epfd << ")\n";

	for (size_t i = 0; i < _servers.size(); i++) {
		if (!_servers[i].getIsSocketOwner())
			continue ;
		
		std::vector<int>& _socketsFDs = _servers[i].getSocketsFDs();
		std::cout << "Server(" << _servers[i].getID() << ") || sockets fds{"; //
		for (size_t x = 0; x < _socketsFDs.size(); x++) {
			_servers[i].setEPFD(_epfd);
			memset(&_event, 0, sizeof(_event));
			_event.data.fd = _socketsFDs[x];
			_event.events = EPOLLIN | EPOLLET; // make the listening socket Edge-triggered 
			
			if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _socketsFDs[x], &_event) == -1) {
				std::cout << "\nepoll ctl failed with server: " << _servers[i].getID() << " socket: " << _socketsFDs[x] << "\n";
				throw "epoll_ctl failed";
			}

			std::cout << _socketsFDs[x]; //
			if ((x + 1) < _socketsFDs.size()) //
				std::cout << ", "; //
		}
		std::cout << "} added to epoll set\n"; //
	}
	std::cout << "---------------------------------------------------------\n";
}


ServerManager::ServerManager(const std::vector<ServerConfig> &serversInfo) : _serversConfig(serversInfo),
																				_2CRLF("\r\n\r\n"), _fileStream("outFile.txt") {
	setUpServers();
	setEpoll();
}

ServerManager::~ServerManager(void) {}