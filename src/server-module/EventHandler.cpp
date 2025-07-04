/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 19:01:35 by karim             #+#    #+#             */
/*   Updated: 2025/07/04 17:35:30 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "Server.hpp"
#include "ServerManager.hpp"

std::vector<int>::iterator	getIterator(int	client_socket, std::vector<int>& sockets) {
	return std::find(sockets.begin(), sockets.end(), client_socket);
}

void	Server::incomingConnection(int NewEvent_fd) {
	int					newClientSocket;
	struct epoll_event	clientEvent;
	ssize_t				clientEventLen = sizeof(clientEvent);
		
	memset(&clientEvent, 0, clientEventLen);
	clientEvent.events = EPOLLIN | EPOLLET; // make the client socket Edge-Triggered
	
	while (true) {
		// since the listening socket is Non-Blocking
		// accept() should make the new return socket Non-Blocking. 
		newClientSocket = accept(NewEvent_fd, NULL, NULL);
		
		try {
			if (newClientSocket == -1) {
				if (errno != EAGAIN && errno != EWOULDBLOCK)
					throw "accept failed";
				else {
					break; // No more pending connections for now
				}
			}
			else {
				
				clientEvent.data.fd = newClientSocket;
				// Add the new client socket to the epoll set to monitor for incoming data (EPOLLIN)
				if (epoll_ctl(_epfd, EPOLL_CTL_ADD, newClientSocket, &clientEvent) == -1)
					throw "epoll_ctl: client_socket failed";
				else {
					// std::cout << "accept ==> " << newClientSocket << "\n";
					_clients[newClientSocket] = Client(newClientSocket, NewEvent_fd); // create a new object where to store the request
					_clientsSockets.push_back(newClientSocket);
				}
			}
		}
		catch (const char *errorMssg) {
			if (newClientSocket != -1)
				close(newClientSocket);
			perror(errorMssg);
		}
	}
}

void	ServerManager::processEvent(Server& server) {
	int clientSocket;

	for (int i = 0; i < _nfds; i++) {
		clientSocket = _events[i].data.fd;
		if (server.verifyServerSocketsFDs(clientSocket)) {
			// std::cout << "########### got an event on the server socket {" << clientSocket << "} ##############\n";
			server.incomingConnection(clientSocket);
		}
		else if (server.verifyClientsFD(clientSocket)) {
			// std::cout << "############  got an event on an existing client socket " << clientSocket << " #############\n";
			server.getClients()[clientSocket].setIncomingDataFlag(INCOMING_DATA_ON);
			server.getClients()[clientSocket].setEvent(_epfd, _events[i]);
		}
	}
}

void    ServerManager::waitingForEvents(void) {
	while (true) {
		// std::cout << "   === >>  Waiting for events...\n";
		_nfds = epoll_wait(_epfd, _events, MAX_EVENTS, EPOLLTIMEOUT);

		if (_nfds < 0)
			throw "epoll_wait failed";
			
		checkTimeOut();
		
		for (size_t x = 0; x < _servers.size(); x++) {
			if (!_servers[x].getIsSocketOwner())
			continue ;
			processEvent(_servers[x]);
			receiveClientsData(x);
			sendClientsResponse(_servers[x]);
		}
	}
}
