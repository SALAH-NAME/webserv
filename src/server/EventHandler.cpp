
#include "ServerManager.hpp"
#include <cerrno>

static void extractPort(std::string& port, uint16_t netPort) {
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

static void	getClientsInfos(ClientInfos* clientinfos, struct HostPort serverInfo, uint32_t rawIP, uint16_t netPort) {
	rawIP = ntohl(rawIP);
	
	ipToString(clientinfos->clientAddr, rawIP);
	extractPort(clientinfos->port, netPort);

	clientinfos->serverInfos.ip = serverInfo.ip;
	clientinfos->serverInfos.port = serverInfo.port;
}

void	ServerManager::incomingConnection(int event_fd) {
	struct sockaddr_in	serverSockAddr;
	struct sockaddr_in	clientAddr;
	socklen_t			serverSockAddrLen = sizeof(serverSockAddr);
	socklen_t			clientAddrLen = sizeof(clientAddr);
	ClientInfos 		clientinfos;
	int					clientSocketFD;

	for (size_t i = 0; i < _listenSockets.size(); i++) {
		if (_listenSockets[i].getFd() != event_fd)
			continue ;

		std::memset(&clientAddr, 0, clientAddrLen);
		std::memset(&serverSockAddr, 0, serverSockAddrLen);
		try {
			while (true) {
				try {
					Socket sock = _listenSockets[i].accept((struct sockaddr*)&clientAddr, &clientAddrLen);
					clientSocketFD = sock.getFd();
					getsockname(clientSocketFD, (struct sockaddr*)&serverSockAddr, &serverSockAddrLen);
					getClientsInfos(&clientinfos, _portsAndHosts[event_fd], clientAddr.sin_addr.s_addr, serverSockAddr.sin_port);
					addSocketToEpoll(_epfd, clientSocketFD, (EPOLLIN | EPOLLHUP | EPOLLERR));
					std::pair<int, Client> entry(clientSocketFD, Client(sock, _serversConfig, _epfd, clientinfos));
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
	
	}

}

bool	ServerManager::processEvent(int i) {
	uint32_t						events;
	std::map<int, Client>::iterator	clientIterator;

	int event_fd = _events[i].data.fd;
	events = _events[i].events;
	
	if (verifyLsteningSocketsFDs(event_fd)) {
		incomingConnection(event_fd);
		return false;
	}
	else if ((clientIterator = verifyClientsFD(event_fd)) != _clients.end()) {
		Client& client = clientIterator->second;
		if (clientIterator->first == event_fd) {
			// event on client socket
			if ((events & EPOLLHUP) || (events & EPOLLERR))
				closeConnection(client);
			else {
				if ((events & EPOLLIN) && client.getInputState() == INPUT_NONE)
					client.setInputState(INPUT_HEADER_READY);
				else
					client.setIsOutputAvailable(events & EPOLLOUT);
				return true;
			}
		}
		else {
			// event on one of client pipes
			if (event_fd == client.getCGI_InpipeFD()) {
				if ((events & EPOLLHUP) || (events & EPOLLERR)) {
					client.closeAndDeregisterPipe(IN_PIPE);
					closeConnection(client);
				}
				else
					client.setIsCgiInputAvailable(events & EPOLLOUT);
			}

			else if (event_fd == client.getCGI_OutpipeFD()
						&& client.getIsCgiRequired() == ON) {
				
				if ((events & EPOLLHUP) && (events & EPOLLIN)) {
					client.setInputState(INPUT_PIPE_HAS_DATA);
					client.setIsCgiRequired(OFF);
				}
				else if (events & EPOLLHUP) {
					client.setInputState(INPUT_PIPE_NO_DATA);
					client.setIsCgiRequired(OFF);
				}
			}
		}
	}
	eraseMarked();
	return false;
}

void    ServerManager::waitingForEvents(void) {

	std::cout << "    ##### BYTES TO SEND : " << BYTES_TO_SEND << "  |||  ";
	std::cout << "BYTES TO READ: " << BYTES_TO_READ << "   ##### \n";

	while (!g_shutdown) {
		std::memset(_events, 0, sizeof(_events));
		_nfds = epoll_wait(_epfd, _events, MAX_EVENTS, EPOLLTIMEOUT);
		if (_nfds < 0) {
			if (errno == EINTR) {
				continue;
			}
			std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
		}
			
		checkTimeOut();

		for (int i = 0; i < _nfds && !g_shutdown; i++) {
			if (!processEvent(i)) // not a client event (pipeFD/listenSocket)
				continue ;
			receiveClientsData(i);
			generatResponses(i);
			sendClientsResponse(i);
		}
	}

	cleanup();
}
