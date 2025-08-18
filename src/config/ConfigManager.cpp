#include "ConfigManager.hpp"
#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"
#include "SimpleLogger.hpp"
#include <iostream>

bool ConfigManager::parseConfigFile()
{
	LOG_DEBUG_F("Parsing configuration file: {}", _config_file);
	
	ConfigTokenizer tokenizer;
	if (!tokenizer.loadFromFile(_config_file))
	{
		LOG_ERROR_F("Failed to load configuration file: {}", _config_file);
		return false;
	}

	ConfigParser parser(tokenizer);
	if (!parser.parse())
	{
		LOG_ERROR_F("Failed to parse configuration file: {}", _config_file);
		return false;
	}

	_global_config = parser.getGlobalConfig();
	_servers			 = parser.getServers();

	if (_servers.empty())
	{
		LOG_ERROR_F("No server configurations found in {}", _config_file);
		return false;
	}

	LOG_INFO_F("Successfully parsed {} server configurations", _servers.size());
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
