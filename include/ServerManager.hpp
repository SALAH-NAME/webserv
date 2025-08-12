#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#define EPOLLTIMEOUT 100
#define MAX_EVENTS 100
#define BYTES_TO_READ 1000 * 1023
#define BYTES_TO_SEND 1000 * 1023
#define BUFFERSIZE (((BYTES_TO_READ > BYTES_TO_SEND) ? BYTES_TO_READ : BYTES_TO_SEND) + 1)
#define RESPONSESIZE 746 // Fix size for the temp response

#define INCOMING_HEADER_DATA_ON true
#define INCOMING_HEADER_DATA_OFF false
#define CONNECTION_ERROR (EPOLLIN | EPOLLERR | EPOLLHUP)

#define INCOMING_BODY_DATA_ON true
#define INCOMING_BODY_DATA_OFF false

#define GENERATE_RESPONSE_ON true
#define GENERATE_RESPONSE_OFF false

#define RESPONSE_HEADER_READY		true
#define RESPONSE_HEADER_NOT_READY	false

#define RESPONSE_BODY_READY		true
#define RESPONSE_BODY_NOT_READY	false

#define FULL_RESPONSE_READY		true
#define FULL_RESPONSE_NOT_READY	false

#define SENDABLE		true
#define NOT_SENDABLE	false

#define WRITABLE		true
#define NOT_WRITABLE	false

#define UPLOAD_ACTIVE   true
#define UPLOAD_NOT_ACTIVE false

#define BODY_DATA_PRELOADED_ON true
#define BODY_DATA_PRELOADED_OFF false

#define REQUEST_DATA_PRELOADED_ON true
#define REQUEST_DATA_PRELOADED_OFF false

#define PIPE_IS_READABLE true
#define PIPE_IS_NOT_READABLE false

#define PIPE_IS_CLOSED true
#define PIPE_IS_NOT_CLOSED false

#define CGI_REQUIRED true
#define CGI_IS_NOT_REQUIRED false

#define READ_PIPE_COMPLETE true
#define READ_PIPE_NOT_COMPLETE false

#define AVAILABLE true
#define NOT_AVAILABLE false

#define SENT true
#define NOT_SENT false

#define PIPE_TO_CGI true
#define NO_PIPE false

#define IS_CHUNKED true
#define NOT_CHUNKED false

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

class Server;
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

struct ClientInfos
{
	std::string clientAddr;
	std::string port;
};


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
		void								collectRequestData(Client&);
		void								transmitResponseHeader(Client&, int);
		void								transferBodyToFile(Client&, int);
		void								transmitFileResponse(Client& , int);
		void								consumeCgiOutput(Client& , int);
		void								transferBodyToCgi(Client& client, int serverIndex);

		void								processEvent(int);
		void								receiveClientsData(int);
		void								generatResponses(int);
		void								sendClientsResponse(int);
		void								handleKeepAlive(Client&, int);

	public:

		/**/								ServerManager(const std::vector<ServerConfig> &);
		/**/								~ServerManager(void);
		void								waitingForEvents(void);

};

void throwIfSocketError(const std::string& context);

#include "Server.hpp"
#include "Client.hpp"
#include "ResponseHandler.hpp"

#endif