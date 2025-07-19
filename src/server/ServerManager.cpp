/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 10:25:03 by karim             #+#    #+#             */
/*   Updated: 2025/07/19 13:13:07 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"

void	ServerManager::generatResponses(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client& client = it->second;
		if (client.getGenerateInProcess() == GENERATE_RESPONSE_OFF)
			continue ;

		it->second.buildResponse();

		client.setResponseInFlight(true);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_OFF);
		// client.prinfRequestinfos();exit(0);
	}

}

void ServerManager::checkTimeOut(void)
{
	for (size_t x = 0; x < _servers.size(); x++)
	{
		std::map<int, Client>& clients = _servers[x].getClients();
		for (std::map<int, Client>::iterator it = clients.begin();
				 it != clients.end(); it++)
		{
			if (std::time(NULL) - it->second.getLastConnectionTime() >
					_servers[x].getTimeOut())
			{
				_servers[x].closeConnection(it->first);
			}
		}
	}
}

void ServerManager::setUpServers(void)
{

	std::cout << "----------------- Set Up Servers ----------------------\n";

	for (size_t i = 0; i < _serversConfig.size(); i++)
	{
		try {
			_servers.push_back(Server(_serversConfig[i], _servers.size() + 1));
		}
		catch (const char* errorMssg) {
			perror(errorMssg);
		}
	}
	if (!_servers.size())
		throw "No server is available";
}

void ServerManager::addToEpollSet(void)
{
	std::cout << "----------------- Add To Epoll Set ----------------------\n";
	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::vector<Socket>& listeningSockets = _servers[i].getListeningSockets();
		if (!listeningSockets.size())
			continue;

		_servers[i].setEPFD(_epfd);

		std::cout << "Server(" << _servers[i].getID() << ") || sockets FDs(" << listeningSockets.size() << ") => {"; //

		for (size_t x = 0; x < listeningSockets.size(); x++)
		{
			memset(&_event, 0, sizeof(_event));
			_event.data.fd = listeningSockets[x].getFd();
			_event.events =
					EPOLLIN | EPOLLET; // make the listening socket Edge-triggered

			if (epoll_ctl(_epfd, EPOLL_CTL_ADD, listeningSockets[x].getFd(),
										&_event) == -1)
			{
				std::cout << "\nepoll ctl failed with server: "
									<< listeningSockets[x].getFd()
									<< " socket: " << listeningSockets[x].getFd() << "\n";
				throw "epoll_ctl failed";
			}

			std::cout << listeningSockets[x].getFd();
			if ((x + 1) < listeningSockets.size())
				std::cout << ", ";
		}
		std::cout << "} added to epoll set\n";
	}
}

void ServerManager::createEpoll()
{
	std::cout << "----------------- Create Epoll ----------------------\n";
	_epfd = epoll_create1(0);
	if (_epfd == -1)
		throw("epoll create1 failed");
	std::cout << "an epoll instance for the servers sockets created(" << _epfd
						<< ")\n";
}

ServerManager::ServerManager(const std::vector<ServerConfig>& serversInfo)
		: _serversConfig(serversInfo), _2CRLF("\r\n\r\n")
{
	createEpoll();
	setUpServers();
	addToEpollSet();
}

ServerManager::~ServerManager(void) {}