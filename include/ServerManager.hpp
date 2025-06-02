#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#define EPOLLTIMEOUT 100
#define MAX_EVENTS 100
#define BYTES_TO_READ 1000
#define IN 1
#define OUT 2

#include "Server.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"

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

											ServerManager(const std::vector<ServerConfig> &serversInfo);
											~ServerManager(void);
		void								waitingForEvents(void);

};

#endif