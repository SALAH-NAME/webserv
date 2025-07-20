#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "ConfigTypes.hpp"
#include <iostream>
#include <set>

void testConfigPrint()
{
	ConfigManager config;
	GlobalConfig& global_config = const_cast<GlobalConfig&>(config.getGlobalConfig());
	std::set<HttpMethod> allowed;
	allowed.insert(HTTP_GET);
	global_config.setAllowedMethods(allowed);


	std::vector<ServerConfig>& servers = const_cast<std::vector<ServerConfig>&>(config.getServers());
	ServerConfig server1;
	server1.addListen(80);
	server1.setHost("127.0.0.1");
	allowed.insert(HTTP_GET);
	allowed.insert(HTTP_POST);
	server1.setAllowedMethods(allowed);

	LocationConfig loc1;
	loc1.setPath("/api", false);
	loc1.setRoot("/var/www/example/api");
	loc1.setClientMaxBodySize("5M");
	loc1.setAutoindex(false);
	server1.addLocation("/api", loc1);

	servers.push_back(server1);

	ConfigPrinter printer(config);

	std::cout << "# Test Configuration" << std::endl;
	std::cout << std::endl;

	printer.print();
}
