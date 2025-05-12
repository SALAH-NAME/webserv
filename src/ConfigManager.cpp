#include "ConfigManager.hpp"

ConfigManager::ConfigManager(const std::string& config_file)
		: _config_file(config_file)
{
}

const GlobalConfig& ConfigManager::getGlobalConfig() const
{
	return _global_config;
}

const std::vector<ServerConfig>& ConfigManager::getServers() const
{
	return _servers;
}
