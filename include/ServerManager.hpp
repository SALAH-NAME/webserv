#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#define EPOLLTIMEOUT 100
#define MAX_EVENTS 100
#define BYTES_TO_READ 1000 * 1023
#define BYTES_TO_SEND 1000 * 1023
#define BUFFERSIZE (((BYTES_TO_READ > BYTES_TO_SEND) ? BYTES_TO_READ : BYTES_TO_SEND) + 1)

#define ON true
#define OFF false

#define IN_PIPE 1
#define OUT_PIPE 0

#define _2CRLF "\r\n\r\n"
#define _CRLF "\r\n"

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
#include <netdb.h>     // for getaddrinfo
#include "HttpRequest.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "Socket.hpp"

extern volatile sig_atomic_t g_shutdown;

class Client;
class ResponseHandler;

enum PostMethodProcessingState {
	DefaultState,			// Do nothing
    ReceivingData,			// Reading from socket into _pendingData
	ValidateChunkSize,		// getting the size of chunked body
    ExtractingBody,			// Parsing body from _pendingData into _requestBodyPart
    UploadingToFile,		// Writing body to target file
    PipingToCGI,			// Streaming body into CGI pipe
    Completed,				// Body fully processed
	CloseConnection,		// when connection closed by peer
	InvalidBody				// Ex: received unexpected data while Expecting CRLF 
};

enum ClientInputState {
    INPUT_PIPE_HAS_DATA,       // CGI pipe closed, data available
    INPUT_PIPE_NO_DATA,        // CGI pipe closed, no data
    INPUT_HEADER_READY,        // Incoming header data
    INPUT_BODY_READY,          // Incoming body data
    INPUT_NONE                 // No input detected
};

struct HostPort
{
	unsigned int	port;
	std::string		ip;

};

struct ClientInfos
{
	std::string clientAddr;
	std::string port;
	struct HostPort serverInfos;
};

class ServerManager {
	private:

		int									_epfd;
		int									_nfds;
		const std::vector<ServerConfig>&	_serversConfig;
		struct epoll_event					_events[MAX_EVENTS];
		char								_buffer[BUFFERSIZE];
		std::vector<Socket>					_listenSockets;
		std::map<int, struct HostPort>		_portsAndHosts;
		std::map<int, Client>				_clients;
		std::vector<int>					_markedForEraseSockets;
		int									_domain;
		int									_timeOut;
		sockaddr_in							_Address;
		bool								_cleanupPerformed;

		void								setup_sockaddr(int, int);
		void								createListenignSockets(int);
		void								closeConnection(Client&);
		void								eraseMarked();
		void								createEpoll(void);
		void								setUpServers(void);
		void    							addToEpollSet(void);
		void								eraseUnusedSockets(void);
		void								checkTimeOut();
		void								collectRequestData(Client&);
		void								transmitResponseHeader(Client&);
		void								transferBodyToFile(Client&);
		void								transmitFileResponse(Client&);
		void								consumeCgiOutput(Client&);
		void								transferBodyToCgi(Client& client, int serverIndex);
		void								incomingConnection(int);
		bool								verifyLsteningSocketsFDs(int);
		std::map<int, Client>::iterator		verifyClientsFD(int);
		void								handleKeepAlive(Client&);
		bool								processEvent(int);
		void								receiveClientsData(int);
		void								generatResponses(int);
		void								sendClientsResponse(int);

		void								cleanup(void);

	public:

		/**/								ServerManager(const std::vector<ServerConfig> &);
		/**/								~ServerManager(void);
		void								waitingForEvents(void);
		void								cleanupChildDescriptors(void);

};

ServerConfig*								getMatchingServerConfig(const std::vector<ServerConfig>& configs, const HttpRequest&, ClientInfos);
void										addSocketToEpoll(int epfd, int fd, uint32_t events);
void										modifyEpollEvents(int epfd, int fd, uint32_t events);
void										deleteEpollEvents(int epfd, int fd);

#include "Client.hpp"
#include "ResponseHandler.hpp"

#endif