/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:39:03 by karim             #+#    #+#             */
/*   Updated: 2025/05/28 16:41:13 by karim            ###   ########.fr       */
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
// #include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"



#define MAX_EVENTS 10
#define BYTES_TO_READ 1000

class Server {
	private:
		static int 							_id;
		int									server_id;
		std::vector<int>					sockets_fds;
		sockaddr_in							_Address;
		std::vector<int>					ports;
		int									_domin;
		int									_type;
		int									_protocol;
		std::string							_host;
		int									_nMaxBacklog; // this parameter is for listen()
		int 								epfd; // epoll fd
		int									nfds; // number of fds in which the event occurred
		unsigned int 						_timeout;
		std::string							_2CRLF;
		bool								_isKeepAlive;

		struct epoll_event					targetInfos; // Structure to define the event we are interested in
		std::vector<struct epoll_event>		events;

		char								buffer[1024];
		size_t								bufferSize;

		std::map<int, Client>				clients;
		std::vector<int>					clientsSockets;
		
		// std::map<int, Response>				responses;
		// std::vector<int>					responseWaitQueue;

		void	__init_attributes(const ServerConfig& _serverInfo);

	public:
		int debug;
		Server(const ServerConfig& _serverInfo);
		~Server(void);

		unsigned int				getTimeout(void);
		int							get_id(void);
		void						setPort(std::vector<int> _port);
		void 						set_epfd(int value);
		struct epoll_event& 		getTarget();
		std::vector<int>&			getSockets_fds();
		void						set_nfds(int value);
		std::vector<int>&			get_clientsSockets(void);
		// std::vector<int>&			get_responseWaitQueue(void);
		void						process_event(struct epoll_event(&tempEvents)[MAX_EVENTS]);
		bool						verifyServerSockets_fds(int NewEvent_fd);
		bool						verifyClientFD(int client_fd);
		void						incomingConnection(int NewEvent_fd);
		void						merge_new_events(struct epoll_event* event);
		
		void    					receiveRequests();
		void						setEventStatus(size_t &i, bool completed);
					
		void    					sendResponses(void);
		
};

void    waitingForEvents(std::vector<Server> &servers, int epfd);

void    printResponse(std::string response);

#endif