/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 19:01:35 by karim             #+#    #+#             */
/*   Updated: 2025/06/02 12:23:02 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

std::vector<int>::iterator	get_iterator(int	client_socket, std::vector<int>& sockets) {
	return std::find(sockets.begin(), sockets.end(), client_socket);
}

void	Server::checkTimeOut(void) {
	for (int i = 0; i < clients.size(); i++) {
		if (std::time(NULL) - clients[clientsSockets[i]].getLastConnectionTime() > serverConfig.getSessionTimeout())
			closeConnection(clientsSockets[i]);
	}
}

void	Server::incomingConnection(int NewEvent_fd) {
	int					newClient_socket;
	struct epoll_event	client_event;
	ssize_t				clientEventLen = sizeof(client_event);
		
	memset(&client_event, 0, clientEventLen);
	client_event.events = EPOLLIN;
	
	while (true) {
		// used accept4() to set the client socket as a Non-Blocking
		newClient_socket = accept4(NewEvent_fd, NULL, NULL, SOCK_NONBLOCK);
		
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
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, newClient_socket, &client_event) == -1)
					throw "epoll_ctl: client_socket failed";
				else {
					// std::cout << "accept ==> " << newClient_socket << "\n";
					clients[newClient_socket] = Client(newClient_socket, NewEvent_fd); // create a new object where to store the request
					clientsSockets.push_back(newClient_socket);
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

void	Server::process_event(struct epoll_event(&events)[MAX_EVENTS], const std::vector<ServerConfig> &serversInfo) {
	int clientSocket;
	
	for (int i = 0; i < nfds; i++) {
		clientSocket = events[i].data.fd;
		if (verifyServerSockets_fds(clientSocket)) {
			// std::cout << "########### got an event on the server socket {" << clientSocket << "} ##############\n";
			incomingConnection(clientSocket);
		}
		else if (verifyClientFD(clientSocket)){
			// std::cout << "############  got an event on an existing client socket " << clientSocket << " #############\n";
			clients[clientSocket].setOutStatus(IN);
			receiveRequests(events[i], serversInfo);
			sendResponses(events[i]);
		}
	}
}

void    waitingForEvents(std::vector<Server> &servers, int epfd, const std::vector<ServerConfig> &serversInfo) {
	int					nfds;
	struct epoll_event	events[MAX_EVENTS];

	while (true) {
		// std::cout << "wait ...\n";
		nfds = epoll_wait(epfd, events, MAX_EVENTS, TIMEOUT);
		
		if (nfds < 0)
			throw "epoll_wait failed";
		
		for (size_t x = 0; x < servers.size(); x++) {
			if (!servers[x].getIsSocketOwner())
				continue ;
			servers[x].checkTimeOut();
			servers[x].set_nfds(nfds);
			servers[x].process_event(events, serversInfo);
		}
		memset(events, 0, sizeof(events));
	}
}