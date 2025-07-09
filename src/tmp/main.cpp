#include "ResponseHandler.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "Request.hpp"

int main()
{
	std::string config_file = "../../conf/webserv.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
	return 1;
	// ConfigPrinter printer(config_manager);
	// printer.print();
	ServerConfig srv_conf = *config_manager.getServers().begin(); 
	std::cout << srv_conf.getRoot() << std::endl;
	ResponseHandler testObj(srv_conf);

}