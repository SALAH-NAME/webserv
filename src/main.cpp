#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"

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

	return 0;
}
