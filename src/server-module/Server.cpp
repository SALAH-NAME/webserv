/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:40:16 by karim             #+#    #+#             */
/*   Updated: 2025/05/13 12:13:03 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(std::map<std::string, int>& infos, std::vector<int>& _ports) {
	int socket_fd;
	_domin = infos["domain"];
	_type = infos["type"];
	_protocol = infos["protocol"];
	server_id = infos["id"];

	ssize_t x = 0;
	for (size_t i = 0; i < _ports.size(); i++, x++) {
		
		try {
			if ((socket_fd = socket(_domin, _type, _protocol)) < 0)
				throw "socket failed: ";

			sockets_fds.push_back(socket_fd);
			
			// if (server_id == 2) {
			// 	std::cout << "\n=========>> here  <=============\n";
			// 	std::cout << "x : " << x << " || sockets_fds size: " << sockets_fds.size() << "\n";
			// 	std::cout << "      socket fd = " << socket_fd << " || push fd: " << sockets_fds[x] << "\n\n";
				
			// }
				

			int reuse = 1;
			setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
			
			_Address.sin_family = _domin;
			_Address.sin_addr.s_addr = _protocol;
			_Address.sin_port = htons(_ports[i]);
			/*htons():  These functions shall convert 16-bit and 32-bit quantities between
						network byte order and host byte order.*/
			
			bufferSize = sizeof(buffer);
			memset(buffer, 0, bufferSize);
			
			if (bind(sockets_fds[x], (sockaddr*)&_Address, sizeof(_Address)) < 0) {
				close(sockets_fds[x]);
				// sockets_fds.erase(sockets_fds.begin() + i);
				sockets_fds.pop_back();
				// std::cout << "size after pop: " << sockets_fds.size() << "\n";
				x--;
				throw "bind failed";
			}
		
			/* _nMaxBacklog: the maximum length to which the queue
				of pending connections for sockfd may grow*/
			if (listen(sockets_fds[x], _nMaxBacklog) < 0)
				throw "listen failed";
		
			std::cout << "Server:" << server_id << " {socket: " << sockets_fds[x] << "} is listening on port " << _ports[i] << "...\n";
		}
		catch (const char* errorMssg) {
			perror(errorMssg);
		}
		
	}

}

Server::~Server(void) {
	// std::cout << "destructor called\n";
}

int		Server::get_id(void) {
	return server_id;
}

void Server::set_epfd(int value) {
	epfd = value;
}

struct epoll_event & Server::getTarget() {
	return targetInfos;
}

std::vector<int>&		Server::getSockets_fds() {
	return sockets_fds;
}

void	Server::set_nfds(int value) {
	nfds = value;	
}

std::map<int, std::time_t>&	Server::get_clientsSockets(void) {
	return clientsSockets;
}

std::vector<int>&	Server::get_responseWaitQueue(void) {
	return responseWaitQueue;
}

bool	Server::verifyClientFD(int client_fd) {
	std::map<int, std::time_t>::iterator it = clientsSockets.find(client_fd);
	return (it != clientsSockets.end());
}

bool	Server::verifyServerSockets_fds(int NewEvent_fd) {
	std::vector<int>::iterator it = std::find(sockets_fds.begin(), sockets_fds.end(), NewEvent_fd);
	return (it != sockets_fds.end());
}