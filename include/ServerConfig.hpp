#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include "BaseConfig.hpp"
#include "LocationConfig.hpp"
#include <string>
#include <vector>
#include <unistd.h>

class ServerConfig : public BaseConfig
{
private:
	std::vector<unsigned int>						 _listen;
	std::string							 _host;
	std::vector<std::string> _server_names;

	unsigned int _connection_timeout;

	std::map<std::string, LocationConfig> _locations;
	std::map<std::string, LocationConfig> _regex_locations;

public:
	ServerConfig(unsigned int connection_timeout = 60);
	void setConnectionTimeout(unsigned int timeout);
	void addListen(unsigned int port);
	void setHost(const std::string& host);
	void addServerName(const std::string& name);
	void setServerNames(const std::vector<std::string>& names);
	void addLocation(const std::string& path, const LocationConfig& location);
	void addRegexLocation(const std::string& regex, const LocationConfig& location);

	unsigned int getConnectionTimeout() const;
	std::vector<unsigned int>										 getListens() const;
	const std::string&													 getHost() const;
	const std::vector<std::string>&							 getServerNames() const;
	const std::map<std::string, LocationConfig>& getLocations() const;
	const std::map<std::string, LocationConfig>& getRegexLocation() const;

	/*const LocationConfig* findLocation(const std::string& path) const;*/
	/*void applayGlobalConfig(const GlobalConfig& GlobalConfig);*/
	/*void applyToLocations();*/
};

#endif
