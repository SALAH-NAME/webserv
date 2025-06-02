// #include "Server.hpp"
#include "ServerManager.hpp"

void	close_fds(std::vector<Server>& servers) {

	for (size_t i = 0; i < servers.size(); i++) {
		int size = servers[i].get_clientsSockets().size();
		for (int x = 0; x < size; x++ )
			close(servers[i].get_clientsSockets()[x]);
	}
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

    try {

		ServerManager	serverManager(config_manager.getServers());
		serverManager.waitingForEvents();

    }
    catch (const char* errorMssg) {
		// close_fds(servers);
		perror(errorMssg);
		return 1;
	}
	return 0;
}