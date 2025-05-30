#include "Server.hpp"

void	close_fds(std::vector<Server>& servers) {

	for (size_t i = 0; i < servers.size(); i++) {
		int size = servers[i].get_clientsSockets().size();
		for (int x = 0; x < size; x++ )
			close(servers[i].get_clientsSockets()[x]);
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
	if (!servers.size())
		throw "No server is available";
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

void    printRequet(std::string requet) {
	for (int i = 0; i < requet.size(); i++) {
		if (requet[i] == '\r')
			std::cout << "\\r";
		else if (requet[i] == '\n')
			std::cout << "\\n\n";
		else
			std::cout << requet[i];
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

    std::vector<Server> servers;
    try {
        setUpServers(servers, config_manager.getServers());
		int epfd = setEpoll(servers);

		waitingForEvents(servers, epfd);
    }
    catch (const char* errorMssg) {
		close_fds(servers);
		perror(errorMssg);
		return 1;
	}
	return 0;
}