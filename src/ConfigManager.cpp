#include "ConfigManager.hpp"
#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"

bool ConfigManager::parseConfigFile()
{
	ConfigTokenizer tokenizer;
	if (!tokenizer.loadFromFile(_config_file))
	{
		std::cerr << "Failed to load configuration file: " << _config_file
							<< std::endl;
		return false;
	}

	ConfigParser parser(tokenizer);
	if (!parser.parse())
	{
		std::cerr << "Failed to parse configuration file: " << _config_file
							<< std::endl;
		return false;
	}

	_global_config = parser.getGlobalConfig();
	_servers			 = parser.getServers();

	if (_servers.empty())
	{
		std::cerr << "No server configurations found in " << _config_file
							<< std::endl;
		return false;
	}

	return true;
}

bool ConfigManager::load() { return parseConfigFile(); }

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
