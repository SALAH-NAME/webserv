/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/05/13 09:29:25 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::setEventStatus(int i, bool completed) {
	// std::cout << "(socket fd: " << socket_fd << ") finished receiving request from fd : " << events[i].data.fd << "\n";
	epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);

	requests.erase(events[i].data.fd);
	
	if (completed) {
		responses[events[i].data.fd] = Response(events[i].data.fd);
		responseWaitQueue.push_back(events[i].data.fd);
	}

	clientsSockets.erase(events[i].data.fd);
	
	events.erase(events.begin() + i);
}

void    Server::receiveRequests() {
	ssize_t bytes_read;

	for (size_t i = 0; i < events.size(); i++) {
		bytes_read = recv(events[i].data.fd, (void *)buffer, 1, 0);
		if (bytes_read > 0) {
			requests[events[i].data.fd].setRequest(buffer);
			clientsSockets[events[i].data.fd] = std::time(NULL);
		}
		if (bytes_read == 0 || requests[events[i].data.fd].getRequest().find("\r\n\r\n") != std::string::npos) {
			// std::cout << "request[" << i << "]: {" << requests[events[i].data.fd].getRequest() << "}\n";
			setEventStatus(i, true);
			i--;
			continue ; 
		}
		if (std::time(NULL) - clientsSockets[events[i].data.fd] > 20) {
			std::cout << "(socket: " << requests[events[i].data.fd].get_serverSocketFD() << ")Time out, close connection";
			std::cout << " with client fd : " << events[i].data.fd << "\n";
			setEventStatus(i, false);
			clientsSockets.erase(events[i].data.fd);
			close(events[i].data.fd);
		}
	}
}