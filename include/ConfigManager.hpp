#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"
#include <string>
class ConfigManager
{
private:
	GlobalConfig							_global_config;
	std::vector<ServerConfig> _servers;
	std::string								_config_file;
	bool											parseConfigFile();

public:
	ConfigManager(const std::string& config_file = "webserv.conf");
	bool														 load();
	const GlobalConfig&							 getGlobalConfig() const;
	const std::vector<ServerConfig>& getServers() const;
	const ServerConfig*							 findServer(const std::string& host,
																							unsigned int			 port) const;
	const ServerConfig* findServer(const std::string& host, unsigned int port,
																 const std::string& server_name) const;
};

#endif
