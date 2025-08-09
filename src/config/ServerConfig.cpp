#include "ServerConfig.hpp"
#include "BaseConfig.hpp"
#include "DefaultConfig.hpp"
#include <ctime>
#include <vector>

ServerConfig::ServerConfig(unsigned int connection_timeout)
    : BaseConfig(), _listen(DefaultConfig::LISTEN), _host(DefaultConfig::HOST),
      _connection_timeout(connection_timeout)
{}

void ServerConfig::addListen(unsigned int port) { _listen.push_back(port); }

void ServerConfig::setHost(const std::string& host) { _host = host; }

void ServerConfig::addServerName(const std::string& name)
{
	_server_names.push_back(name);
}

void ServerConfig::setServerNames(const std::vector<std::string>& names)
{
	_server_names = names;
}

void ServerConfig::addLocation(const std::string&		 path,
															 const LocationConfig& location)
{
	_locations[path] = location;
}

void ServerConfig::addRegexLocation(const std::string&		regex,
																		const LocationConfig& location)
{
	_regex_locations[regex] = location;
}

std::vector<unsigned int> ServerConfig::getListens() const { return _listen; }

const std::string& ServerConfig::getHost() const { return _host; }

const std::vector<std::string>& ServerConfig::getServerNames() const
{
	return _server_names;
}

const std::map<std::string, LocationConfig>& ServerConfig::getLocations() const
{
	return _locations;
}

const std::map<std::string, LocationConfig>&
ServerConfig::getRegexLocation() const
{
	return _regex_locations;
}

unsigned int ServerConfig::getConnectionTimeout() const
{
	return _connection_timeout;
}

void ServerConfig::setConnectionTimeout(unsigned int timeout)
{
	_connection_timeout = timeout;
}