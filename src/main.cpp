#include "Server.hpp"

void	close_fds(std::vector<Server>& servers) {

	for (size_t i = 0; i < servers.size(); i++) {
		
		std::map<int, std::time_t> &clientsSockets = servers[i].get_clientsSockets();
		for (size_t x = 0; x < clientsSockets.size(); x++)
			close(clientsSockets[x]);

		std::vector<int>	&responseWaitQueue = servers[i].get_responseWaitQueue();
		for (size_t x = 0; x < responseWaitQueue.size(); x++)
			close(responseWaitQueue[x]);
	}
}

void signal_handler(int sig) {
	if (sig == SIGINT) {
		throw "\nSingal called";
	}
}

std::vector<std::map<std::string, int> >& getInfos(int serverNum) {

	if (serverNum > 4) {
		std::cout << "set more ports\n";
		exit(0);
	}
	
	std::vector<std::map<std::string, int> > &serverInfos = *(new std::vector<std::map<std::string, int> >);
	std::vector<int> ports;
	
	ports.push_back(8080);
	ports.push_back(5050);
	ports.push_back(1010);
	ports.push_back(4747);
	
	for (int i = 0; i < serverNum; i++) {

		std::map<std::string, int> newInfos;
		
		newInfos["type"] = SOCK_STREAM | SOCK_NONBLOCK;
		newInfos["protocol"] = 0;
		newInfos["nMaxBacklog"] = 100;
		newInfos["port"] = ports[i];
		newInfos["domain"] = AF_INET;
		serverInfos.push_back(newInfos);
	}
	return serverInfos;
}

void	setUpServers(std::vector<Server>& servers, int serversNum) {
	std::vector<std::map<std::string, int> >& serverInfos = getInfos(serversNum);

	std::cout << "starting....\n";
	for (int i = 0; i < serversNum; i++) {
		try {
			servers.push_back(Server(serverInfos[i]));
		}
		catch (const char *errorMssg) {
			perror(errorMssg);
		}
	}
	delete &serverInfos;
}

int	setEpoll(std::vector<Server> &servers) {
	int epfd = epoll_create1(0);
	if (epfd == -1)
		throw ("epoll create1 failed");
	std::cout << "an epoll instance for the servers sockets created\n";

	for (size_t i = 0; i < servers.size(); i++) {

		servers[i].set_epfd(epfd);
		struct epoll_event	&targetInfos = servers[i].getTarget();
		targetInfos.data.fd = servers[i].getSocket_fd();  // The file descriptor we are interested in (the server socket)
		targetInfos.events = EPOLLIN;     // We are interested in read events (incoming connections)
		targetInfos.events |= EPOLLET;    // Use Edge-Triggered mode for efficiency

		if (epoll_ctl(epfd, EPOLL_CTL_ADD, servers[i].getSocket_fd(), &targetInfos) == -1)
			throw "epoll_ctl failed";
		std::cout << "Server socket {" << servers[i].getSocket_fd() << "} added to epoll set." << std::endl;
	}
	return epfd;
}

int main(int argc, char* argv[])
{
	(void) argc, (void) argv;
	std::cout << "HELLO TO WEB SERVER ~~~!" << std::endl;

    int serversNum = 4;
    std::vector<Server> servers;

    try {
        setUpServers(servers, serversNum);
		int epfd = setEpoll(servers);

        std::cout << "------------------------------------------------\n";
        signal(SIGINT, signal_handler);

		waitingForEvents(servers, epfd);
    }
    catch (const char* errorMssg) {
		if (!std::strcmp(errorMssg, "\nSingal called")) {
			close_fds(servers);
			std::cout << errorMssg;
		}
		else
	        perror(errorMssg);
    }
	return 0;
}