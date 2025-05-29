/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/05/29 14:29:47 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::setEventStatus(size_t &i, bool completed) {

	// requests.erase(events[i].data.fd);
	
	if (completed) {
		// responses[events[i].data.fd] = Response(events[i].data.fd);
		// responseWaitQueue.push_back(events[i].data.fd);

		clients[events[i].data.fd].setOutStatus(true);

		events[i].events = EPOLLIN | EPOLLOUT;  // enable write temporarily
		epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]);

	}
	else {
		epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
		clientsSockets.erase(clientsSockets.begin() + i);	
		close(events[i].data.fd);
		clients.erase(events[i].data.fd);
		events.erase(events.begin() + i);
		i--;
	}
}

void    Server::receiveRequests() {
	ssize_t bytes_read;

	// std::cout << "event size: " << events.size() << "\n";
	for (size_t i = 0; i < events.size(); i++) {
		bytes_read = recv(events[i].data.fd, (void *)buffer, BYTES_TO_READ, 0);
		if (bytes_read > 0) {
			clients[events[i].data.fd].setRequest(buffer);
			clients[events[i].data.fd].setReadBytes(bytes_read);
			// clientsSockets[events[i].data.fd] = std::time(NULL);
		}
		if (bytes_read == 0 || clients[events[i].data.fd].getRequest().find(_2CRLF) != std::string::npos) {
			// printResponse(requests[events[i].data.fd].getRequest());
			setEventStatus(i, true);
			debug = 1;
			continue ; 
		}
		// if (std::time(NULL) - clientsSockets[events[i].data.fd] > getTimeout()) {
		// 	std::cout << "(socket: " << requests[events[i].data.fd].get_serverSocketFD() << ")Time out, close connection";
		// 	std::cout << " with client fd : " << events[i].data.fd << "\n";
		// 	setEventStatus(i, false);
		// 	clientsSockets.erase(events[i].data.fd);
		// 	close(events[i].data.fd);
		// }
	}
}