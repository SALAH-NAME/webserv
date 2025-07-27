#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#define EPOLLTIMEOUT 100
#define MAX_EVENTS 100
#define BYTES_TO_READ 1023
#define BYTES_TO_SEND 1023
#define BUFFERSIZE 1024
#define RESPONSESIZE 746 // Fix size for the temp response

#define INCOMING_HEADER_DATA_ON true
#define INCOMING_DATA_OFF false
#define CONNECTION_ERROR (EPOLLIN | EPOLLERR | EPOLLHUP)

#define GENERATE_RESPONSE_ON true
#define GENERATE_RESPONSE_OFF false

#define TRANSFER_BODY_ON true
#define TRANSFER_BODY_OFF false

#define BODY_DATA_PRELOADED_ON true
#define BODY_DATA_PRELOADED_OFF false

#define RESPONSE_READY      true   // Response Data is Enough To Send
#define RESPONSE_PENDING    false  // Still collecting data, not ready yet

#define GET_RESPONSE_ON   true
#define GET_RESPONSE_OFF  false

#define RESPONSE_SEND_DONE     true
#define RESPONSE_SEND_PENDING  false

#define _2CRLF "\r\n\r\n"

#include <netinet/in.h> // For sockaddr_in
#include <unistd.h> 
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
#include <cerrno>
#include <map>
#include <sstream>
#include <algorithm>
#include <arpa/inet.h> // for inet_addr()
#include <utility>
#include "HttpRequest.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "Socket.hpp"
#include "ResponseHandler.hpp"

class Server;
class Client;

class ServerManager {
	private:

		int									_epfd;
		int									_nfds;
		std::vector<Server>					_servers;
		const std::vector<ServerConfig>&	_serversConfig;
		struct epoll_event					_event;
		struct epoll_event					_events[MAX_EVENTS];
		char								_buffer[BUFFERSIZE];

		void								createEpoll(void);
		void								setUpServers(void);
		void    							addToEpollSet(void);
		void								checkTimeOut(void);
		void								collectRequestData(Client&, int);
		void								transmitResponse(Client&, int);
		void								transferBodyToFile(Client&, int);
		void								transmitFileResponse(Client& , int);

		void								processEvent(int);
		void								receiveClientsData(int);
		void								generatResponses(int);
		void								sendClientsResponse(int);

	public:

		/**/								ServerManager(const std::vector<ServerConfig> &);
		/**/								~ServerManager(void);
		void								waitingForEvents(void);


};

void throwIfSocketError(const std::string& context);

#include "Server.hpp"
#include "Client.hpp"

#endif