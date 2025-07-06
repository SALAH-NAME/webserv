#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#define EPOLLTIMEOUT 100
#define MAX_EVENTS 100
#define BYTES_TO_READ 1000
// #define BYTES_TO_SEND 1000
#define BUFFERSIZE 1024
// #define RESPONSESIZE 746 // Fix size for the temp response

#define CONNECTION_ERROR (EPOLLIN | EPOLLERR | EPOLLHUP)


// #define INCOMING_DATA_ON true
// #define INCOMING_DATA_OFF false

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
#include "HttpRequest.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "Socket.hpp"

class Server;

class ServerManager {
	private:

		int									_epfd;
		int									_nfds;
		std::vector<Server>					_servers;
		const std::vector<ServerConfig>&	_serversConfig;
		struct epoll_event					_event;
		struct epoll_event					_events[MAX_EVENTS];
		char								_buffer[BUFFERSIZE];
		std::string							_2CRLF;

		void								createEpoll(void);
		void								setUpServers(void);
		void    							addToEpollSet(void);
		void								checkTimeOut(void);
		void								collectRequestData(Client&, int);

		void								processEvent(int);
		void								receiveClientsData(int);
		void								sendClientsResponse(int);

	public:

											ServerManager(const std::vector<ServerConfig> &);
											~ServerManager(void);
		void								waitingForEvents(void);


};

#include "Server.hpp"

#endif