/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:39:03 by karim             #+#    #+#             */
/*   Updated: 2025/06/03 12:29:01 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

// #include <netinet/in.h> // For sockaddr_in
// #include <unistd.h>     // For close()
// #include <iostream>
// #include <string>
// #include <vector>
// #include <fcntl.h>
// #include <sys/epoll.h>
// #include <map>
// #include <cstring>
// #include <csignal>
// #include <cstdlib>
// #include <cstdio>
// #include <ctime>
// #include <cerrno> // should be removed
// #include <map>
// #include <sstream>
// #include <algorithm>
// #include <arpa/inet.h> // for inet_addr()

// #include "Client.hpp"
// #include "Response.hpp"

// #include "ConfigManager.hpp"
// #include "ConfigPrinter.hpp"
// #include "HttpRequest.hpp"

#include "ServerManager.hpp"

class Server {
	private:
		const ServerConfig&		_serverConfig;
		static int 				_id;
		int						_server_id;
		std::vector<int>		_sockets_fds;
		sockaddr_in				_Address;
		std::vector<int>		_ports;
		int						_domin;
		int						_type;
		int						_protocol;
		int						_nMaxBacklog; // this parameter is for listen()
		int 					_epfd; // epoll fd
		std::string				_2CRLF;
		bool					_isKeepAlive;
		bool					_isSocketOwner;
		char					_buffer[1024];
		size_t					_bufferSize;
		int						_timeOut;

		std::map<int, Client>	_clients;
		std::vector<int>		_clientsSockets;

		void					__init_attributes(void);
		void					setEventStatus(struct epoll_event& event, int completed);


	public:
								Server(const ServerConfig&);
								~Server(void);

		int						get_id(void);
		void					setPort(std::vector<int> _port);
		void 					set__epfd(int value);
		std::vector<int>&		get_sockets_fds();
		std::vector<int>&		get_clientsSockets(void);
		bool					verifyServer_sockets_fds(int NewEvent_fd);
		bool					verifyClientFD(int client_fd);
		void					incomingConnection(int NewEvent_fd);			
		void					checkTimeOut(void);
		bool					getIsSocketOwner(void);
		void					closeConnection(int clientSocket);
		void    				receiveRequests(struct epoll_event& event);
		void					sendResponses(struct epoll_event& event);
		int						getTimeOut(void);
		std::map<int, Client>&	getClients(void);
		
		
		
};

std::vector<int>::iterator	get_iterator(int	client_socket, std::vector<int>& sockets);
void						printRequet(std::string Requet);

#endif