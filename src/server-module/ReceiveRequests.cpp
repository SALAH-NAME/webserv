/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/05/30 15:11:53 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::setEventStatus(struct epoll_event& event, int completed) {
	int clientSocket = event.data.fd;

	if (completed) {
		clients[clientSocket].setOutStatus(true);
		event.events = EPOLLIN | EPOLLOUT;  // enable write temporarily
		epoll_ctl(epfd, EPOLL_CTL_MOD, clientSocket, &event);

	}
	else
		closeConnection(clientSocket);
}

void    Server::receiveRequests(struct epoll_event& event) {
	ssize_t bytes_read;
	int clientSocket = event.data.fd;
		
	if (clients[clientSocket].getOutStatus() != IN)
		return ;
	bytes_read = recv(clientSocket, (void *)buffer, BYTES_TO_READ, 0);
	if (bytes_read > 0) {
		clients[clientSocket].setRequest(buffer);
		clients[clientSocket].setReadBytes(bytes_read);
		clients[clientSocket].resetLastConnectionTime();
	}
	if (bytes_read == 0 || clients[clientSocket].getRequest().find(_2CRLF) != std::string::npos) {
		// printRequet(clients[clientSocket].getRequest());
		setEventStatus(event, true);
	}
}