
#include "ServerManager.hpp"
#include <csignal>

volatile sig_atomic_t g_shutdown = 0;

void signalHandler(int signal)
{
	if (signal == SIGINT)
	{
		std::cout << "\n\nReceived SIGINT (Ctrl+C). Shutting down gracefully...\n" << std::endl;
		g_shutdown = 1;
	}
}

void    printRequestAndResponse(std::string str, std::string request) {
	char lasChar = 0;
	std::cout << "\n********************  " << str << "  *********************\n";
	for (size_t i = 0; i < request.size(); i++) {
		if (request[i] == '\r')
			std::cout << "\\r";
		else if (request[i] == '\n')
			std::cout << "\\n\n";
		else
			std::cout << request[i];
		lasChar = request[i];
	}
	if (lasChar != '\n')
		std::cout << "\n";
	std::cout << "********** " << str << " size: " << request.size() << " **************\n\n";
}

int main(int argc, char** argv)
{
	signal(SIGINT, signalHandler);

	std::string config_file = "conf/default.conf";
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
		std::cerr << errorMssg;
		return 1;
	}
	return 0;
}
