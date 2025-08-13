
#include "Server.hpp"

void	Server::initAttributes(int id) {
	_domain = AF_INET;
	_type = SOCK_STREAM | SOCK_NONBLOCK;
	_protocol = 0;
	_ports = _serverConfig.getListens();
	_timeOut = _serverConfig.getConnectionTimeout();
	_id = id;
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

void	Server::setup_sockaddr(int port) {
    _Address.sin_family = _domain;              // AF_INET
    _Address.sin_port = htons(port);           // Desired port

    struct in_addr resolvedAddr;
    resolveIPv4(_serverConfig.getHost(), resolvedAddr);       // Resolve IP
    _Address.sin_addr = resolvedAddr;          // Set resolved IPr;
}

Server::Server(const std::vector<ServerConfig>& allServersConfig, const ServerConfig& serverConfig, size_t id) :
				_allServersConfig(allServersConfig), _serverConfig(serverConfig), _id(id), _transferSocket(){
	initAttributes(id);

	for (size_t i = 0; i < _ports.size(); i++) {
		
		try {
			_listeningSockets.push_back(Socket());
		
			_listeningSockets[_listeningSockets.size() - 1].create();
			
			int reuse = 1;
			_listeningSockets[_listeningSockets.size() - 1].setsockopt(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
			// fixed this problem ==>  the OS keeps the port in a "cool-down" period (TIME_WAIT)
			// ==> It’s mainly for quick restart development or for binding during graceful restarts.

			setup_sockaddr(_ports[i]);

			_listeningSockets[_listeningSockets.size() - 1].bind(reinterpret_cast<sockaddr*>(&_Address), sizeof(struct sockaddr));
			_listeningSockets[_listeningSockets.size() - 1].listen();
			std::cout << "Server(" << _id << ") {socket: " << _listeningSockets[_listeningSockets.size() - 1].getFd() << "} is listening on => ";
			std::cout << serverConfig.getHost() << ":" << _ports[i] << "\n";
		}
		catch (const std::runtime_error& e) {
			_listeningSockets.pop_back();
			perror(e.what());
		}
	}
	if (!_listeningSockets.size())
		throw "server failed";
}

Server::~Server(void) {}

const ServerConfig&	Server::getConfig(void) {
	return _serverConfig;
}

int		Server::getID(void) {
	return _id;
}

const std::vector<unsigned int>	Server::getPorts(void) {
	return _ports;
}

std::vector<int>	Server::getMarkedForEraseClients() {
	return _markedForEraseClients;
}

void Server::setEPFD(int value) {
	_epfd = value;
}

std::vector<Socket>&		Server::getListeningSockets() {
	return _listeningSockets;
}

std::vector<int>& Server::getMarkedForEraseUnusedSocket(void) {
	return _markedForEraseUnusedClient;
}

std::map<int, Client>::iterator	Server::verifyClientsFD(int client_fd) {
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if (client_fd == it->first || client_fd == it->second.getCGI_OutpipeFD() || client_fd == it->second.getCGI_InpipeFD())
			return it ;
	}
	return _clients.end();
}

bool	Server::verifyServerSocketsFDs(int NewEvent_fd) {
	for (size_t i = 0; i < _listeningSockets.size(); i++) {
		if (NewEvent_fd == _listeningSockets[i].getFd())
			return true;
	}
	return false;
}

void	Server::closeConnection(int clientSocket) {
	epoll_ctl(_epfd, EPOLL_CTL_DEL, clientSocket, NULL);
	_markedForEraseClients.push_back(clientSocket);
}

void	Server::eraseMarked() {
	for (size_t i = 0; i < _markedForEraseClients.size(); i++) {
		close(_markedForEraseClients[i]);
		std::cout << "close connection: " << _markedForEraseClients[i] << "\n";
		_clients.erase(_markedForEraseClients[i]);
	}
	_markedForEraseClients.clear();
}

std::map<int, Client>& Server::getClients() {
	return _clients;
}

int	Server::getTimeOut(void) {
	return _timeOut;
}

Socket&	Server::getTransferSocket(void) {
	return _transferSocket;
}
