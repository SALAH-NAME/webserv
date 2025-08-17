
#include "ServerManager.hpp"

void addSocketToEpoll(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		throw std::runtime_error("     + epoll_ctl(ADD) failed +");
	}
}

void modifyEpollEvents(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1)
        throw std::runtime_error("epoll_ctl(MOD) failed");
}

void deleteEpollEvents(int epfd, int fd) {
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1)
        throw std::runtime_error("epoll_ctl(MOD) failed");
}

void	ServerManager::generatResponses(int i) {

	std::map<int, Client>::iterator it = _clients.find(_events[i].data.fd);
	if (it == _clients.end())
		return ;
	Client& client = it->second;
	

	if (client.getGenerateInProcess() == OFF)
		return ;
	
	try {
		it->second.buildResponse();
	} catch(std::runtime_error& e) {
		closeConnection(client);
		perror(e.what());
		return ;
	}
	catch(const char* msg)
	{
		std::cerr << msg << std::endl;
		return ;
	}
	client.setGenerateResponseInProcess(OFF);
	eraseMarked();
}

void	ServerManager::closeConnection(Client& client) {
	int clientFD = client.getSocket().getFd();

	if (client.getIsCgiRequired() && client.getResponseHandler()->GetCgiChildPid())
		kill(client.getResponseHandler()->GetCgiChildPid(), SIGKILL);
	if (client.getIsCgiRequired()) {
		if (client.getCGI_InpipeFD() != -1)
			close(client.getCGI_InpipeFD());
		if (client.getCGI_OutpipeFD() != -1)
			close(client.getCGI_OutpipeFD());
	}

	epoll_ctl(_epfd, EPOLL_CTL_DEL, clientFD, NULL);
	_markedForEraseSockets.push_back(clientFD);
}

void	ServerManager::eraseMarked() {
	for (std::vector<int>::const_iterator fdIt = _markedForEraseSockets.begin(); fdIt != _markedForEraseSockets.end(); ++fdIt) {
        std::map<int, Client>::iterator clientIt = _clients.find(*fdIt);
        if (clientIt != _clients.end())
            _clients.erase(clientIt);
    }
	_markedForEraseSockets.clear();
}

void ServerManager::checkTimeOut()
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		Client& client = it->second;
		if (std::time(NULL) - client.getLastConnectionTime() > _timeOut)
		{
			std::cout << "\n    ==>> * Connection Time out: " << _timeOut << "  * <<==\n\n";
			closeConnection(client);
		}

		if (client.getIsCgiRequired()) {
			try {
				client.getResponseHandler()->CheckCgiChildState();
			} catch (ResponseHandler::ResponseHandlerError& e) {
				std::cout << "\n    ==>>  * CGI Time Out *  <<==\n\n";
				std::cout << e.what() << std::endl;
				client.getResponseHandler()->LoadErrorPage(e.what(), e.getStatusCode());
				client.CgiExceptionHandler();
			}
		}
	}
	eraseMarked();
}

bool	ServerManager::verifyLsteningSocketsFDs(int event_fd) {
	for (size_t i = 0; i < _listenSockets.size(); i++) {
		if (event_fd == _listenSockets[i].getFd())
			return true;
	}
	return false;
}

std::map<int, Client>::iterator		ServerManager::verifyClientsFD(int client_fd) {
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if (client_fd == it->first || client_fd == it->second.getCGI_OutpipeFD() || client_fd == it->second.getCGI_InpipeFD())
			return it ;
	}
	return _clients.end();
}

static void resolveIPv4(const std::string& host, struct in_addr& outAddr) {
    struct addrinfo hints, *res = NULL;

    std::memset(&hints, 0, sizeof(hints));

    int status = getaddrinfo(host.c_str(), NULL, &hints, &res);
    if (status != 0 || res == NULL)
        throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(status)));

    // Extract the IPv4 address
    struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    outAddr = ipv4->sin_addr;

    freeaddrinfo(res);
}

void	ServerManager::setup_sockaddr(int configIndex, int port) {
    _Address.sin_family = _domain;              // AF_INET
    _Address.sin_port = htons(port);           // Desired port

    struct in_addr resolvedAddr;
    resolveIPv4(_serversConfig[configIndex].getHost(), resolvedAddr);       // Resolve IP
    _Address.sin_addr = resolvedAddr;          // Set resolved IPr;
}

void	ServerManager::createListenignSockets(int configIndex) {
	std::vector<unsigned int> ports = _serversConfig[configIndex].getListens();

	for (size_t i = 0; i < ports.size(); i++) {
		_listenSockets.push_back(Socket());
		Socket& socket = _listenSockets.back();
		try {
			socket.create();
			
			int reuse = 1;
			socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
			// fixed this problem ==>  the OS keeps the port in a "cool-down" period (TIME_WAIT)
			// ==> It’s mainly for quick restart development or for binding during graceful restarts.
			setup_sockaddr(configIndex, ports[i]);
			socket.bind(reinterpret_cast<sockaddr*>(&_Address), sizeof(struct sockaddr));
			socket.listen();
			
			HostPort hp;
			hp.ip = _serversConfig[configIndex].getHost();
			hp.port = ports[i];
			_portsAndHosts.insert(std::make_pair(socket.getFd(), hp));
			
			std::cout << "Server(" << (configIndex + 1) << ") {socket: " << socket << "} is listening on => ";
			std::cout << _serversConfig[configIndex].getHost() << ":" << ports[i] << "\n";

		} catch (std::runtime_error& e) {
			_listenSockets.pop_back();
			perror(e.what());
		}
	}
}

void ServerManager::setUpServers(void) {

	std::cout << "----------------- Set Up Servers ----------------------\n";
	for (size_t i = 0; i < _serversConfig.size(); i++)
	{
		if (_timeOut != -1)
			_timeOut = _serversConfig[i].getConnectionTimeout();

		createListenignSockets(i);
	}
	if (!_listenSockets.size())
		throw "    ===>>>> Can't run the program, None of the servers is running!! <<<<===\n";
}

void ServerManager::addToEpollSet(void) {

	std::cout << "----------------- Add To Epoll Set ----------------------\n";


	int AddedSockets = 0;
	int listenSocketFD;
	for (size_t i = 0; i < _listenSockets.size(); i++) {
		try {
			listenSocketFD = _listenSockets[i].getFd();
			addSocketToEpoll(_epfd, listenSocketFD, (EPOLLIN | EPOLLET)); // make the listening socket Edge-triggered
			AddedSockets++;
		} catch (const std::runtime_error& e) {
			_markedForEraseSockets.push_back(listenSocketFD);
			perror(e.what());
			continue ;
		}
	}
	if (!AddedSockets)
		throw "    ===>>>> Can't run the program, None of the servers is running!! <<<<===\n";
}

void ServerManager::createEpoll() {

	std::cout << "----------------- Create Epoll ----------------------\n";
	_epfd = epoll_create(1);
	if (_epfd == -1)
		throw("epoll create1 failed");
	std::cout << "an epoll instance for the servers sockets created(" << _epfd
						<< ")\n";
}

void ServerManager::eraseUnusedSockets() {
	for (std::vector<int>::const_iterator fdIt = _markedForEraseSockets.begin(); fdIt != _markedForEraseSockets.end(); ++fdIt) {
        for (std::vector<Socket>::iterator sockIt = _listenSockets.begin(); sockIt != _listenSockets.end(); ) {
            if (sockIt->getFd() == *fdIt)
                sockIt = _listenSockets.erase(sockIt);
            else
                ++sockIt;
        }
	}
	_markedForEraseSockets.clear();
}

ServerManager::ServerManager(const std::vector<ServerConfig>& serversInfo)
		: _serversConfig(serversInfo), _domain(AF_INET), _type(SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC)
		,  _timeOut(1), _cleanupPerformed(false) {
	
	createEpoll();
	setUpServers();
	addToEpollSet();
	eraseUnusedSockets();
}

ServerManager::~ServerManager(void) {
	cleanup();
}

void ServerManager::cleanup(void) {
	if (_cleanupPerformed)
		return;
	
	std::cout << "Starting server cleanup..." << std::endl;
	
	// close all client
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		Client& client = it->second;
		// kill cgi child
		if (client.getIsCgiRequired() && client.getResponseHandler()->GetCgiChildPid()) {
			kill(client.getResponseHandler()->GetCgiChildPid(), SIGTERM);
			for (int i = 0; i < 10000; ++i) { } // sleep for a short time
			kill(client.getResponseHandler()->GetCgiChildPid(), SIGKILL);
		}

		// cgi pipes
		if (client.getIsCgiRequired()) {
			if (client.getCGI_InpipeFD() != -1) {
				epoll_ctl(_epfd, EPOLL_CTL_DEL, client.getCGI_InpipeFD(), NULL);
				close(client.getCGI_InpipeFD());
			}
			if (client.getCGI_OutpipeFD() != -1) {
				epoll_ctl(_epfd, EPOLL_CTL_DEL, client.getCGI_OutpipeFD(), NULL);
				close(client.getCGI_OutpipeFD());
			}
		}
		
		// remove client from epoll
		int clientFD = client.getSocket().getFd();
		epoll_ctl(_epfd, EPOLL_CTL_DEL, clientFD, NULL);
	}
	
	/// clean clients
	_clients.clear();
	
	// close listen sockets
	for (std::vector<Socket>::iterator it = _listenSockets.begin(); it != _listenSockets.end(); ++it) {
		int sockFD = it->getFd();
		epoll_ctl(_epfd, EPOLL_CTL_DEL, sockFD, NULL);
	}
	_listenSockets.clear();
	
	/// close epoll fd
	if (_epfd != -1) {
		close(_epfd);
		_epfd = -1;
	}
	
	_cleanupPerformed = true;
	std::cout << "Server cleanup completed." << std::endl;
}

void	ServerManager::cleanupChildDescriptors(void) {
	for (size_t i = 0; i < _listenSockets.size(); i++) {
		_listenSockets[i].close();
	}

	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		int CGI_InPipe = it->second.getCGI_InpipeFD();
		int CGI_OutPipe = it->second.getCGI_OutpipeFD();
		if (CGI_InPipe != -1) {
			close(CGI_InPipe);
			it->second.setCgiInputPipe(-1);
		}
		if (CGI_OutPipe != -1) {
			close(CGI_OutPipe);
			it->second.setCgiOutPipe(-1);
		}
		it->second.getSocket().close();
	}

	close(_epfd);
}