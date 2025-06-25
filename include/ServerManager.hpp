#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#define EPOLLTIMEOUT -1
#define MAX_EVENTS 100
#define BYTES_TO_READ 1

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
#include "HttpRequest.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"

class Server;

class ServerManager {
	private:

		int									_epfd;
		int									_nfds;
		std::vector<Server>					_servers;
		const std::vector<ServerConfig>&	_serversConfig;
		struct epoll_event					_event;
		struct epoll_event					_events[MAX_EVENTS];

		void								setUpServers(void);
		void    							setEpoll(void);
		void								checkTimeOut(void);
		void								process_event(Server&);

	public:

											ServerManager(const std::vector<ServerConfig> &);
											~ServerManager(void);
		void								waitingForEvents(void);

};

#include "Server.hpp"

#endif