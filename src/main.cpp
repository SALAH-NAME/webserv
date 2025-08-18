
#include "ServerManager.hpp"
#include "SimpleLogger.hpp"
#include <csignal>

volatile sig_atomic_t g_shutdown = 0;

void signalHandler(int signal)
{
	if (signal == SIGINT)
	{
		LOG_WARN("Received SIGINT (Ctrl+C). Shutting down...");
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

	SET_LOG_LEVEL(LEVEL_DEBUG);
	LOG_INFO("WebServer starting up...");

	std::string config_file = "conf/default.conf";
	if (argc > 1)
	{
		config_file = argv[1];
		LOG_INFO_F("Using custom configuration file: {}", config_file);
	}
	else
	{
		LOG_INFO_F("Using default configuration file: {}", config_file);
	}

	ConfigManager config_manager(config_file);
	if (!config_manager.load())
	{
		LOG_ERROR_F("Failed to load configuration from: {}", config_file);
		return 1;
	}
	LOG_INFO("Configuration loaded successfully");

	// ConfigPrinter printer(config_manager);
	// printer.print();

    try {
		LOG_INFO("Initializing server manager...");
		ServerManager	serverManager(config_manager.getServers());
		LOG_INFO("Server manager initialized, waiting for events...");
		serverManager.waitingForEvents();

    }
    catch (const char* errorMssg) {
		LOG_ERROR_F("Server error: {}", errorMssg);
		return 1;
	}
	LOG_INFO("WebServer shutdown complete");
	return 0;
}
