/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 19:01:35 by karim             #+#    #+#             */
/*   Updated: 2025/06/28 13:05:20 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "Server.hpp"
#include "ServerManager.hpp"

std::vector<int>::iterator	get_iterator(int	client_socket, std::vector<int>& sockets) {
	return std::find(sockets.begin(), sockets.end(), client_socket);
}

void	Server::incomingConnection(int NewEvent_fd) {
	int					newClient_socket;
	struct epoll_event	client_event;
	ssize_t				clientEventLen = sizeof(client_event);
		
	memset(&client_event, 0, clientEventLen);
	client_event.events = EPOLLIN;
	
	while (true) {
		// since the listening socket is Non-Blocking
		// acccept() should make the new return socket Non-Blocking. 
		newClient_socket = accept(NewEvent_fd, NULL, NULL);
		
		try {
			if (newClient_socket == -1) {
				if (errno != EAGAIN && errno != EWOULDBLOCK)
					throw "accept failed";
				else {
					break; // No more pending connections for now
				}
			}
			else {
				
				client_event.data.fd = newClient_socket;
				// Add the new client socket to the epoll set to monitor for incoming data (EPOLLIN)
				if (epoll_ctl(_epfd, EPOLL_CTL_ADD, newClient_socket, &client_event) == -1)
					throw "epoll_ctl: client_socket failed";
				else {
					std::cout << "accept ==> " << newClient_socket << "\n";
					_clients[newClient_socket] = Client(newClient_socket, NewEvent_fd); // create a new object where to store the request
					_clientsSockets.push_back(newClient_socket);
				}
			}
		}
		catch (const char *errorMssg) {
			if (newClient_socket != -1)
				close(newClient_socket);
			perror(errorMssg);
		}
	}
}

void	ServerManager::process_event(Server& server) {
	int clientSocket;
	
	if (_nfds)
		std::cout << "==========> Detected an event  <=========\n";
	for (int i = 0; i < _nfds; i++) {
		clientSocket = _events[i].data.fd;
		if (server.verifyServer_sockets_fds(clientSocket)) {
			std::cout << "########### got an event on the server socket {" << clientSocket << "} ##############\n";
			server.incomingConnection(clientSocket);
		}
		else if (server.verifyClientFD(clientSocket)){
			std::cout << "############  got an event on an existing client socket " << clientSocket << " #############\n";
			
			server.receiveRequests(_events[i]);
			server.sendResponses(_events[i]);
		}
	}
}

void    ServerManager::waitingForEvents(void) { 
	while (true) {
		std::cout << "[INFO] Waiting for events...\n";
		_nfds = epoll_wait(_epfd, _events, MAX_EVENTS, EPOLLTIMEOUT);

		if (_nfds < 0)
			throw "epoll_wait failed";
		
		checkTimeOut();

		for (size_t x = 0; x < _servers.size(); x++) {
			if (!_servers[x].getIsSocketOwner())
				continue ;
			process_event(_servers[x]);
		}
	}
}