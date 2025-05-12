#include "ServerConfig.hpp"
#include "BaseConfig.hpp"
#include "DefaultConfig.hpp"
#include <ctime>
#include <vector>

ServerConfig::ServerConfig()
		: BaseConfig(), _listen(DefaultConfig::LISTEN), _host(DefaultConfig::HOST),
			_session_enable(DefaultConfig::SESION_ENABLE),
			_session_timeout(DefaultConfig::SESSION_TIMEOUT)
{
}

void ServerConfig::setListen(unsigned int port) { _listen = port; }

void ServerConfig::setHost(const std::string& host) { _host = host; }

void ServerConfig::addServerName(const std::string& name)
{
	_server_names.push_back(name);
}

void ServerConfig::setServerNames(const std::vector<std::string>& names)
{
	_server_names = names;
}

void ServerConfig::setSessionEnable(bool enalbe) { _session_enable = enalbe; }

void ServerConfig::setSessionName(const std::string& name)
{
	_session_name = name;
}

void ServerConfig::setSessionPath(const std::string& path)
{
	_session_path = path;
}

void ServerConfig::setSessionTimeout(unsigned int timeout)
{
	_session_timeout = timeout;
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

unsigned int ServerConfig::getListen() const { return _listen; }

const std::string& ServerConfig::getHost() const { return _host; }

const std::vector<std::string>& ServerConfig::getServerNames() const
{
	return _server_names;
}

bool ServerConfig::getSessionEnable() const { return _session_enable; }

const std::string& ServerConfig::getSessionName() const
{
	return _session_name;
}

const std::string& ServerConfig::getSessionPath() const
{
	return _session_path;
}

unsigned int ServerConfig::getSessionTimeout() const
{
	return _session_timeout;
}

const std::map<std::string, LocationConfig>& ServerConfig::getLocation() const
{
	return _locations;
}

const std::map<std::string, LocationConfig>&
ServerConfig::getRegexLocation() const
{
	return _regex_locations;
}
