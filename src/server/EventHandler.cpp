
#include "ServerManager.hpp"

static void extractPort(std::string& port, uint16_t netPort) {
    // Network byte order is big-endian: high byte first
	uint16_t hostPort = ((netPort >> 8) & 0xFF) | ((netPort & 0xFF) << 8);
    
    std::stringstream ss;
    ss << hostPort;
    port = ss.str();
}


static void	ipToString(std::string& ip, uint32_t rawAddr) {
    std::stringstream ss;

    ss << ((rawAddr >> 24) & 0xFF) << '.'
        << ((rawAddr >> 16) & 0xFF) << '.'
        << ((rawAddr >> 8) & 0xFF) << '.'
        << (rawAddr & 0xFF);

    ip = ss.str();
}

static void	getClientsInfos(ClientInfos* clientinfos, uint32_t rawIP, uint16_t netPort) {
	// Convert it to host byte order
	rawIP = ntohl(rawIP);
	
	ipToString(clientinfos->clientAddr, rawIP);
	extractPort(clientinfos->port, netPort);
}


void	Server::incomingConnection(int NewEvent_fd) {
	struct sockaddr_in	serverSockAddr;
	struct sockaddr_in	clientAddr;
	socklen_t			serverSockAddrLen = sizeof(serverSockAddr);
	socklen_t			clientAddrLen = sizeof(clientAddr);
	ClientInfos 		clientinfos;
	int					clientSocketFD;

	for (size_t i = 0; i < _listeningSockets.size(); i++) {
		if (_listeningSockets[i].getFd() != NewEvent_fd)
			continue ;
		
		std::memset(&clientAddr, 0, clientAddrLen);
		std::memset(&serverSockAddr, 0, serverSockAddrLen); 
		try {
			while (true) {
				try {
					Socket sock = _listeningSockets[i].accept((struct sockaddr*)&clientAddr, &clientAddrLen);
					clientSocketFD = sock.getFd();
					getsockname(clientSocketFD, (struct sockaddr*)&serverSockAddr, &serverSockAddrLen);
					getClientsInfos(&clientinfos, clientAddr.sin_addr.s_addr, serverSockAddr.sin_port);
					// std::cout << "  ======>>> accept : " << clientSocketFD << " <<====== \n";
					addSocketToEpoll(_epfd, clientSocketFD, (EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET)); // make the client socket Edge-Triggered

					std::pair<int, Client> entry(clientSocketFD, Client(sock, _serverConfig, _allServersConfig, _epfd, clientinfos));
					_clients.insert(entry);
				}
				catch (const char *errorMssg) {
					break ; // no more pending connections
				}
			}
		}
		catch (const std::runtime_error& e) {
			perror(e.what());
			break ; // some sys calls failed
		}
		break ; // once found the listening socket where even came from accept evens on that socket and break
	}
}

void	ServerManager::processEvent(int serverIndex) {
	int								event_fd;
	uint32_t						events;
	Server&							server =_servers[serverIndex];
	std::map<int, Client>::iterator	clientIterator;

	for (int i = 0; i < _nfds; i++) {
		event_fd = _events[i].data.fd;
		events = _events[i].events;
		
		// std::cout << "  ### Event on: " << event_fd << " ###\n";
		if (server.verifyServerSocketsFDs(event_fd)) {
			// std::cout << "########### got an event on the server socket {" << event_fd << "} ##############\n";
			server.incomingConnection(event_fd);
		}
		else if ((clientIterator = server.verifyClientsFD(event_fd)) != server.getClients().end()) {
			// std::cout << "############  got an event on an existing client socket or pipe fd " << event_fd << " #############\n";
			Client& client = clientIterator->second;
			if (clientIterator->first == event_fd) {
				if ((events & EPOLLHUP) || (events & EPOLLERR)) {
					server.closeConnection(client);
					continue ;
				}
				if (client.getIncomingBodyDataDetectedFlag() == INCOMING_BODY_DATA_OFF)
					client.setIncomingHeaderDataDetectedFlag(INCOMING_HEADER_DATA_ON);

				client.setIsOutputAvailable(events & EPOLLOUT);
			}
			else {
				// event on one of client pipes
				
				if (event_fd == client.getCGI_InpipeFD()) {
					if ((events & EPOLLHUP) || (events & EPOLLERR)) { // if an error occured
						client.closeAndDeregisterPipe(event_fd);
						_servers[serverIndex].closeConnection(client);
						client.setState(DefaultState);
					}
					else
						client.setIsCgiInputAvailable(events & EPOLLOUT); // check if PIPE is available for writing
				}

				else if (event_fd == client.getCGI_OutpipeFD()
							&& client.getIsCgiRequired() == CGI_REQUIRED) { // check if PIPE is ready from reading
					
					if ((events & EPOLLHUP) && (events & EPOLLIN)) {
						// std::cout << "set pipe to \"PIPE_IS_CLOSED WITH INPUT\" (ready to read)\n";
						client.setIsPipeClosedByPeer(PIPE_IS_CLOSED);
						client.setIsCgiRequired(CGI_IS_NOT_REQUIRED);
					}
					else if (events & EPOLLHUP) {
						// std::cout << "set pipe to \"PIPE_IS_CLOSED WITH NO INPUT\n";
						client.setIsPipeClosedByPeer(PIPE_CLOSED_NO_INPUT);
						client.setIsCgiRequired(CGI_IS_NOT_REQUIRED);
					}
				}
			}
		}
	}
	server.eraseMarked();
}

void    ServerManager::waitingForEvents(void) {

	std::cout << "    ##### BYTES TO SEND : " << BYTES_TO_SEND << "  |||  ";
	std::cout << "BYTES TO READ: " << BYTES_TO_READ << "   ##### \n";

	while (true) {
		std::memset(_events, 0, sizeof(_events));
		_nfds = epoll_wait(_epfd, _events, MAX_EVENTS, EPOLLTIMEOUT);
		if (_nfds < 0)
			throw "epoll_wait failed";
			
		checkTimeOut();
		
		for (size_t x = 0; x < _servers.size(); x++) {
			if (!_servers[x].getListeningSockets().size())
				continue ;

			processEvent(x);			
			receiveClientsData(x);
			generatResponses(x);
			sendClientsResponse(x);
		}
	}
}
