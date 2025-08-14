
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

void	ServerManager::generatResponses(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client& client = it->second;

		if (client.getGenerateInProcess() == GENERATE_RESPONSE_OFF)
			continue ;
		
		try {
			it->second.buildResponse();
		}
		catch(std::runtime_error& e) {
			_servers[serverIndex].closeConnection(client);
			perror(e.what());
			continue ;
		}
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_OFF);
	}
	_servers[serverIndex].eraseMarked();
}

void ServerManager::checkTimeOut(void)
{
	for (size_t x = 0; x < _servers.size(); x++)
	{
		std::map<int, Client>& clients = _servers[x].getClients();
		for (std::map<int, Client>::iterator it = clients.begin();
				 it != clients.end(); it++)
		{
			if (std::time(NULL) - it->second.getLastConnectionTime() >
					_servers[x].getTimeOut())
			{
				std::cout << "Time out: " << _servers[x].getTimeOut() << "\n";
				_servers[x].closeConnection(it->second);
			}

			if (it->second.getIsCgiRequired()) {
				try {
					it->second.getResponseHandler()->CheckCgiChildState();
				} catch (ResponseHandler::ResponseHandlerError& e) {
					std::cout << "------> the exception was caught by the server <------" << std::endl;
					std::cout << e.what() << std::endl;
					it->second.getResponseHandler()->LoadErrorPage(e.what(), e.getStatusCode());
					it->second.CgiExceptionHandler();
				}
			}
		}
		_servers[x].eraseMarked();
	}
}

void ServerManager::setUpServers(void) {

	std::cout << "----------------- Set Up Servers ----------------------\n";

	for (size_t i = 0; i < _serversConfig.size(); i++)
	{
		try {
			_servers.push_back(Server(_serversConfig, _serversConfig[i], (_servers.size() + 1)));
		}
		catch (const char* errorMssg) {
			perror(errorMssg);
		}
	}
	if (!_servers.size())
		throw "    ===>>>> Can't run the program, None of the servers is running!! <<<<===\n";
}

void ServerManager::addToEpollSet(void) {

	std::cout << "----------------- Add To Epoll Set ----------------------\n";
	int runningServers = 0;
	for (size_t i = 0; i < _servers.size(); i++) {
		int AddedSockets = 0;
		std::vector<Socket>& listeningSockets = _servers[i].getListeningSockets();
		if (!listeningSockets.size())
			continue;

		_servers[i].setEPFD(_epfd);

		for (size_t x = 0; x < listeningSockets.size(); x++) {
			std::memset(&_event, 0, sizeof(_event)); // std

			try {
				addSocketToEpoll(_epfd, listeningSockets[x].getFd(), (EPOLLIN | EPOLLET)); // make the listening socket Edge-triggered
				AddedSockets++;
			} catch (const std::runtime_error& e) {
				_servers[i].getMarkedForEraseUnusedSocket().push_back(listeningSockets[x].getFd());
				perror(e.what());
				continue ;
			}
		}
		try {
			if (!AddedSockets)  {
				throw " * server is not Available, None if its sockets is added to epoll set *";
			} 
			else
				runningServers++;
		} catch (const char* errorMssg) {
			std::cerr << errorMssg << "\n\n";
			continue ;
		}
	}
	if (!runningServers)
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

void ServerManager::printRunningServers(void) {
    std::cout << "----------------- Running Servers -----------------------\n";

    for (size_t i = 0; i < _servers.size(); ++i) {
		int serverID = _servers[i].getID();
        const std::vector<Socket>& ListeningSockets = _servers[i].getListeningSockets();

        std::cout << "Server[" << serverID << "]: sockets nums(" << ListeningSockets.size() << ") ==> {";  
		
        for (size_t j = 0; j < ListeningSockets.size(); ++j) {
			std::cout << ListeningSockets[j].getFd();
            if (j + 1 < ListeningSockets.size())
			std::cout << ", ";
        }
		std::cout << "}\n";
    }
}

void ServerManager::eraseUnusedSockets() {

	for (size_t i = 0; i < _servers.size(); i++) {

		std::vector<Socket>&	listeningSockets = _servers[i].getListeningSockets();
		std::vector<int>&		markedForEraseUnusedClient = _servers[i].getMarkedForEraseUnusedSocket();

    	for (size_t x = 0; x < listeningSockets.size(); ) {
    	    int fd = listeningSockets[x].getFd();
    	    bool shouldErase = false;

    	    for (size_t j = 0; j < markedForEraseUnusedClient.size(); ++j) {
    	        if (markedForEraseUnusedClient[j] == fd) {
    	            shouldErase = true;
    	            break;
    	        }
    	    }

    	    if (shouldErase) {
    	        listeningSockets.erase(listeningSockets.begin() + x);
    	    } else {
    	        ++x;
    	    }
    	}
		markedForEraseUnusedClient.clear();
	}
}

ServerManager::ServerManager(const std::vector<ServerConfig>& serversInfo)
		: _serversConfig(serversInfo) {

	createEpoll();
	setUpServers();
	addToEpollSet();
	eraseUnusedSockets();
	printRunningServers();
}

ServerManager::~ServerManager(void) {}
