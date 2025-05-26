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

void	setUpServers(std::vector<Server>& servers, const std::vector<ServerConfig> &serversInfo) {

	for (size_t i = 0; i < serversInfo.size(); i++) {
		try {
			servers.push_back(Server(serversInfo[i]));
			if (!servers.back().getSockets_fds().size())
				servers.pop_back();
		}
		catch (const char *errorMssg) {
			perror(errorMssg);
		}
	}
}

int	setEpoll(std::vector<Server> &servers) {
	std::cout << "----------------- Set Epoll ----------------------\n";
	int epfd = epoll_create1(0);
	if (epfd == -1)
		throw ("epoll create1 failed");
	std::cout << "an epoll instance for the servers sockets created(" << epfd << ")\n";

	for (size_t i = 0; i < servers.size(); i++) {

		std::vector<int>& sockets_fds = servers[i].getSockets_fds();
		std::cout << "Server(" << servers[i].get_id() << ") || sockets fds{"; //
		for (size_t x = 0; x < sockets_fds.size(); x++) {
			servers[i].set_epfd(epfd);
			struct epoll_event	&targetInfos = servers[i].getTarget();
			targetInfos.data.fd = sockets_fds[x];
			targetInfos.events = EPOLLIN;
			targetInfos.events |= EPOLLET;
			
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
	std::cout << "---------------------------------------------------------\n";
	return epfd;
}

void    printResponse(std::string response) {
	for (int i = 0; i < response.size(); i++) {
		if (response[i] == '\r')
			std::cout << "\\r";
		else if (response[i] == '\n')
			std::cout << "\\n\n";
		else
			std::cout << response[i];
	}
}

int main(int argc, char** argv)
{

	std::string config_file = "conf/webserv.conf";
	if (argc > 1)
	{
		config_file = argv[1];
	}

	ConfigManager config_manager(config_file);
	if (!config_manager.load())
	{
		return 1;
	}

	ConfigPrinter printer(config_manager);
	printer.print();

	std::cout << "==============================\n";

    std::vector<Server> servers;
    try {
        setUpServers(servers, config_manager.getServers());
		int epfd = setEpoll(servers);

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