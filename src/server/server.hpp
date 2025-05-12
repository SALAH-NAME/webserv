/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:39:03 by karim             #+#    #+#             */
/*   Updated: 2025/05/12 19:22:31 by karim            ###   ########.fr       */
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

#include "request.hpp"
#include "response.hpp"

#define MAX_EVENTS 10

class Server {
	private:
		int									socket_fd;
		sockaddr_in							_Address;
		int									port;
		int									_domin;
		int									_type;
		int									_protocol;
		int									_nMaxBacklog; // this parameter is for listen()
		int 								epfd; // epoll fd
		int									nfds; // number of fds in which the event occurred

		struct epoll_event					targetInfos; // Structure to define the event we are interested in
		std::vector<struct epoll_event>		events;

		char								buffer[1024];
		size_t								bufferSize;

		std::map<int, std::time_t>			clientsSockets;
		std::map<int, Request>				requests;
		std::map<int, Response>				responses;
		std::vector<int>					responseWaitQueue;

	public:
		Server(std::map<std::string, int>&);
		~Server(void);

		void set_epfd(int value);
		struct epoll_event & getTarget();
		int		getSocket_fd();
		void	set_nfds(int value);
		std::map<int, std::time_t>&			get_clientsSockets(void);
		std::vector<int>&					get_responseWaitQueue(void);
		void	process_event(struct epoll_event(&tempEvents)[MAX_EVENTS]);
		bool	verifyClientFD(int client_fd);
		void	incomingConnection();
		void	merge_new_events(struct epoll_event* event);
		
		void    receiveRequests();
		void	setEventStatus(int i, bool completed);

		void    sendResponses(void);
		
};

void    waitingForEvents(std::vector<Server> &servers, int epfd);

#endif