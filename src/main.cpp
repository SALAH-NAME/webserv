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

std::vector<std::map<std::string, int> > getInfos(int serverNum) {
	
	std::vector<std::map<std::string, int> > serverInfos;
	
	for (int i = 0; i < serverNum; i++) {

		std::map<std::string, int> newInfos;
		
		newInfos["type"] = SOCK_STREAM | SOCK_NONBLOCK;
		newInfos["protocol"] = 0;
		newInfos["nMaxBacklog"] = 100;
		newInfos["domain"] = AF_INET;
		newInfos["id"] = i + 1;
		serverInfos.push_back(newInfos);
	}
	return serverInfos;
}

std::vector<std::vector<int> >	setUpPorts(void) {
	std::vector<int> ports1;
	std::vector<int> ports2;
	std::vector<int> ports3;
	std::vector<int> ports4;

	
	ports1.push_back(8080);
	ports1.push_back(5050);
	ports2.push_back(1010);
	ports2.push_back(4747);
	ports3.push_back(1212);
	ports3.push_back(5757);
	ports4.push_back(6699);
	ports4.push_back(7070);

	std::vector<std::vector<int> > Ports;
	Ports.push_back(ports1);
	Ports.push_back(ports2);
	Ports.push_back(ports3);
	Ports.push_back(ports4);

	return (Ports);
}

void	setUpServers(std::vector<Server>& servers, int serversNum) {
	if (serversNum > 4) {
		std::cout << "set more ports\n";
		exit(0);
	}

	std::vector<std::map<std::string, int> > serverInfos = getInfos(serversNum);
	std::vector<std::vector<int> > Ports = setUpPorts();

	std::cout << "starting....\n";
	for (int i = 0; i < serversNum; i++) {
		try {
			servers.push_back(Server(serverInfos[i], Ports[i]));
		}
		catch (const char *errorMssg) {
			perror(errorMssg);
		}
	}
	setUpPorts();
}

int	setEpoll(std::vector<Server> &servers) {
	std::cout << "----------------- Set Epoll ----------------------\n";
	int epfd = epoll_create1(0);
	if (epfd == -1)
		throw ("epoll create1 failed");
	std::cout << "an epoll instance for the servers sockets created\n";

	for (size_t i = 0; i < servers.size(); i++) {

		std::vector<int>& sockets_fds = servers[i].getSockets_fds();
		// std::cout << "server " << servers[i].get_id() << " size: " << servers[i].getSockets_fds().size() << "\n";
		std::cout << "Server: " << servers[i].get_id() << " || sockets fds{"; //
		for (size_t x = 0; x < sockets_fds.size(); x++) {
			servers[i].set_epfd(epfd);
			struct epoll_event	&targetInfos = servers[i].getTarget();
			targetInfos.data.fd = sockets_fds[x];  // The file descriptor we are interested in (the server socket)
			targetInfos.events = EPOLLIN;     // We are interested in read events (incoming connections)
			targetInfos.events |= EPOLLET;    // Use Edge-Triggered mode for efficiency
			
			if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockets_fds[x], &targetInfos) == -1) {
				std::cout << "\nepoll ctl failed with server: " << servers[i].get_id() << " socket: " << sockets_fds[x] << "\n";
				throw "epoll_ctl failed";
			}

			std::cout << sockets_fds[x]; //
			if ((x + 1) < sockets_fds.size()) //
				std::cout << ", "; //
		}
		std::cout << "} added to epoll set\n"; //
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