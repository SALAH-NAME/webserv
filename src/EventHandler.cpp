/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 19:01:35 by karim             #+#    #+#             */
/*   Updated: 2025/05/19 10:35:13 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::merge_new_events(struct epoll_event* newEvent) {
	for (size_t i = 0; i < events.size(); i++) {
		if (events[i].data.fd == newEvent->data.fd) {
			// this event already merged, still WIP
			return ;
		}
	}
	
	events.push_back(*newEvent); // add the new event to the previous ones
}

void	Server::incomingConnection(int NewEvent_fd) {

	struct sockaddr_in	client_address;
	socklen_t			addrLen = sizeof(client_address);
	int					newClient_fd;
	struct epoll_event	client_event;
	ssize_t				clientEventLen = sizeof(client_event);
		
	memset(&client_address, 0, addrLen);
	memset(&client_event, 0, clientEventLen);
	client_event.events = EPOLLIN | EPOLLOUT;

	while (true) {
		// used accept4() to set the client socket as a Non-Blocking
		newClient_fd = accept4(NewEvent_fd, (struct sockaddr*)&client_address, &addrLen, SOCK_NONBLOCK);
		try {
			if (newClient_fd == -1) {
				if (errno != EAGAIN && errno != EWOULDBLOCK)
					throw "accept failed";
				else
					break; // No more pending connections for now
			}
			else {				
				client_event.data.fd = newClient_fd;
				
				// Add the new client socket to the epoll set to monitor for incoming data (EPOLLIN)
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, newClient_fd, &client_event) == -1) {
					close(newClient_fd);
					throw "epoll_ctl: client_socket failed";
				}
				else {
					requests[newClient_fd] = Request(newClient_fd, NewEvent_fd); // create a new object where to store the request
					clientsSockets[newClient_fd] = std::time(NULL);
				}
			}
		}
		catch (const char *errorMssg) {
			if (newClient_fd != -1)
				close(newClient_fd);
			perror(errorMssg);
		}
	}
}

void	Server::process_event(struct epoll_event(&tempEvents)[MAX_EVENTS]) {
	for (int i = 0; i < nfds; i++) {
		if (verifyServerSockets_fds(tempEvents[i].data.fd)) {
			// std::cout << "########### got an event on the server socket ##############\n";
			incomingConnection(tempEvents[i].data.fd);
		}
		else if (verifyClientFD(tempEvents[i].data.fd)){
			// std::cout << "############  got an event on an existing client socket #############\n";
			merge_new_events(&tempEvents[i]);
		}
	}
}

void    waitingForEvents(std::vector<Server> &servers, int epfd) {
	int		nfds;
	struct epoll_event	tempEvents[MAX_EVENTS];

	while (true) {

		// std::cout << "Waiting for new events ...\n";
		nfds = epoll_wait(epfd, tempEvents, MAX_EVENTS, -1);

		if (nfds < 0)
			throw "epoll_wait failed";

		for (size_t x = 0; x < servers.size(); x++) {
			servers[x].set_nfds(nfds);
			servers[x].process_event(tempEvents);
			servers[x].receiveRequests();
			servers[x].sendResponses();
		}
		memset(tempEvents, 0, sizeof(tempEvents));
	}
}