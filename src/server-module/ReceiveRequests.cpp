/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/06/25 19:44:20 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerManager.hpp"

void	Server::setEventStatus(struct epoll_event& event, int completed) {
	int clientSocket = event.data.fd;

	if (completed) {
		event.events = EPOLLIN | EPOLLOUT;  // enable write temporarily
		epoll_ctl(_epfd, EPOLL_CTL_MOD, clientSocket, &event);
	}
	else
		closeConnection(clientSocket);
}

void    Server::receiveRequests(struct epoll_event& event) {
	ssize_t bytes_read;
	int clientSocket = event.data.fd;

	if (event.events == EPOLLOUT)
		return ;
	bytes_read = recv(clientSocket, (void *)_buffer, BYTES_TO_READ, 0);
	if (bytes_read > 0) {
		_clients[clientSocket].setRequest(_buffer);
		_clients[clientSocket].setReadBytes(bytes_read);
		_clients[clientSocket].resetLastConnectionTime();
	}
	if (bytes_read == 0 || _clients[clientSocket].getRequest().find(_2CRLF) != std::string::npos) {
		// printRequet(clients[clientSocket].getRequest());
		if (!_clients[clientSocket].parseRequest())
			setEventStatus(event, false);
		else
			setEventStatus(event, true);
	}
}