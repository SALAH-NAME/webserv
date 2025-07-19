/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 19:01:35 by karim             #+#    #+#             */
/*   Updated: 2025/07/19 13:45:14 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "Server.hpp"
#include "ServerManager.hpp"

void throwIfSocketError(const std::string& context) {
	switch (errno) {
		case EAGAIN:
			// Non-fatal: try again later
			break;
		case EINTR:
			// Retry-able signal interruption
			break;
		case ECONNRESET:
		case ENOTCONN:
		case EPIPE:
		case EBADF:
			throw std::runtime_error(context + ": fatal socket error - " + strerror(errno));
		default:
			throw std::runtime_error(context + ": unknown socket error - " + strerror(errno));
	}
}

void	Server::incomingConnection(int NewEvent_fd) {
	struct epoll_event	clientEvent;
	ssize_t				clientEventLen = sizeof(clientEvent);
	int					clientSocketFD;
		
	memset(&clientEvent, 0, clientEventLen);
	clientEvent.events = EPOLLIN | EPOLLET; // make the client socket Edge-Triggered
	
	for (size_t i = 0; i < _listeningSockets.size(); i++) {
		if (_listeningSockets[i].getFd() != NewEvent_fd)
			continue ;
		
		try {
			while (true) {
				try {
					Socket sock = _listeningSockets[i].accept();
					clientSocketFD = sock.getFd();
					// std::cout << "accept : " << clientSocketFD << "\n";
					clientEvent.data.fd = clientSocketFD;
					if (epoll_ctl(_epfd, EPOLL_CTL_ADD, clientSocketFD, &clientEvent) == -1)
						throw std::runtime_error(std::string("epoll_ctl() failed: ") + strerror(errno));
					Client newClient(sock, NewEvent_fd, _serverConfig); // create a new object where to store the request
					_clients.insert(std::make_pair(clientSocketFD, newClient));
				}
				catch (const char *errorMssg) {
					break ; // no more pending connections
				}
			}
		}
		catch (const std::runtime_error& e) {
			perror(e.what());
			break ; // some sys calls failed
		}
		break ; // once found the listening socket where even came from accept evens on that socket and break
	}
}

void	ServerManager::processEvent(int serverIndex) {
	int clientSocket;
	Server& server =_servers[serverIndex];

	for (int i = 0; i < _nfds; i++) {
		clientSocket = _events[i].data.fd;
		if (server.verifyServerSocketsFDs(clientSocket)) {
			// std::cout << "########### got an event on the server socket {" << clientSocket << "} ##############\n";
			server.incomingConnection(clientSocket);
		}
		else if (server.verifyClientsFD(clientSocket)) {
			// std::cout << "############  got an event on an existing client socket " << clientSocket << " #############\n";
			if (_events[i].events == CONNECTION_ERROR) {
				server.closeConnection(_events[i].data.fd);
				continue ;
			}
			std::map<int, Client>::iterator clientIterator = server.getClients().find(clientSocket);
			clientIterator->second.setIncomingDataFlag(INCOMING_DATA_ON);
			clientIterator->second.setEvent(_epfd, _events[i]);
		}
	}
	server.eraseMarked();
}

void    ServerManager::waitingForEvents(void) {
	int i = 0;
	while (true) {
		_nfds = epoll_wait(_epfd, _events, MAX_EVENTS, EPOLLTIMEOUT);
		if (_nfds < 0)
			throw "epoll_wait failed";
			
		checkTimeOut();
		
		for (size_t x = 0; x < _servers.size(); x++) {
			if (!_servers[x].getListeningSockets().size())
				continue ;

			processEvent(x);
			receiveClientsData(x);
			generatResponses(x);
			sendClientsResponse(x);
		}
		i++;
	}
}
