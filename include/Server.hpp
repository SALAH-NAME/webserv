/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:39:03 by karim             #+#    #+#             */
/*   Updated: 2025/06/02 12:24:30 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/epoll.h>
#include <map>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cerrno> // should be removed
#include <map>
#include <sstream>
#include <algorithm>
#include <arpa/inet.h> // for inet_addr()

#include "Client.hpp"
#include "Response.hpp"

#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "HttpRequest.hpp"


#define TIMEOUT 100
#define MAX_EVENTS 100
#define BYTES_TO_READ 1000
#define IN 1
#define OUT 2

class Server {
	private:
		const ServerConfig&					serverConfig;
		static int 							_id;
		int									server_id;
		std::vector<int>					sockets_fds;
		sockaddr_in							_Address;
		std::vector<int>					ports;
		int									_domin;
		int									_type;
		int									_protocol;
		int									_nMaxBacklog; // this parameter is for listen()
		int 								epfd; // epoll fd
		int									nfds; // number of fds in which the event occurred
		std::string							_2CRLF;
		bool								_isKeepAlive;
		bool								_isSocketOwner;
		char								buffer[1024];
		size_t								bufferSize;

		std::map<int, Client>				clients;
		std::vector<int>					clientsSockets;

		void								__init_attributes(const std::vector<ServerConfig> &serversInfo, int i);
		void								setEventStatus(struct epoll_event& event, int completed, const std::vector<ServerConfig> &serversInfo);
		void    							receiveRequests(struct epoll_event& event, const std::vector<ServerConfig> &serversInfo);
		void								sendResponses(struct epoll_event& event);
		void								closeConnection(int clientSocket);
		const std::vector<ServerConfig>&	getServersInfo(void);


	public:
		Server(const std::vector<ServerConfig> &serversInfo, int i);
		~Server(void);

		int							get_id(void);
		void						setPort(std::vector<int> _port);
		void 						set_epfd(int value);
		std::vector<int>&			getSockets_fds();
		void						set_nfds(int value);
		std::vector<int>&			get_clientsSockets(void);
		void						process_event(struct epoll_event(&tempEvents)[MAX_EVENTS], const std::vector<ServerConfig> &serversInfo);
		bool						verifyServerSockets_fds(int NewEvent_fd);
		bool						verifyClientFD(int client_fd);
		void						incomingConnection(int NewEvent_fd);			
		void    					sendResponses(struct epoll_event(&tempEvents)[MAX_EVENTS]);
		void						checkTimeOut(void);
		bool						getIsSocketOwner(void);
		
};

void						waitingForEvents(std::vector<Server> &servers, int epfd, const std::vector<ServerConfig> &serversInfo);
std::vector<int>::iterator	get_iterator(int	client_socket, std::vector<int>& sockets);

void						printRequet(std::string Requet);

#endif