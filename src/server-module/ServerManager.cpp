/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 10:25:03 by karim             #+#    #+#             */
/*   Updated: 2025/06/25 19:44:35 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"

void	ServerManager::checkTimeOut(void) {

	for (int x = 0; x < _servers.size(); x++) {
		
		std::map<int, Client>&	clients = _servers[x].getClients();
		std::vector<int>&		clientsSockets = _servers[x].get_clientsSockets();
		
		for (ssize_t i = 0; i < clients.size(); i++) {
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
		
		std::vector<int>& _sockets_fds = _servers[i].get_sockets_fds();
		std::cout << "Server(" << _servers[i].get_id() << ") || sockets fds{"; //
		for (size_t x = 0; x < _sockets_fds.size(); x++) {
			_servers[i].set__epfd(_epfd);
			memset(&_event, 0, sizeof(_event));
			_event.data.fd = _sockets_fds[x];
			_event.events = EPOLLIN | EPOLLOUT;
			
			if (epoll_ctl(_epfd, EPOLL_CTL_ADD, _sockets_fds[x], &_event) == -1) {
				std::cout << "\nepoll ctl failed with server: " << _servers[i].get_id() << " socket: " << _sockets_fds[x] << "\n";
				throw "epoll_ctl failed";
			}

			std::cout << _sockets_fds[x]; //
			if ((x + 1) < _sockets_fds.size()) //
				std::cout << ", "; //
		}
		std::cout << "} added to epoll set\n"; //
	}
	std::cout << "---------------------------------------------------------\n";
}


ServerManager::ServerManager(const std::vector<ServerConfig> &serversInfo) : _serversConfig(serversInfo) {
    setUpServers();
	setEpoll();
}

ServerManager::~ServerManager(void) {}