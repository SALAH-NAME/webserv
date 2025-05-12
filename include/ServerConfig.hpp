#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include "BaseConfig.hpp"
#include "LocationConfig.hpp"
#include <string>
#include <vector>

class ServerConfig : public BaseConfig
{
private:
	unsigned int						 _listen;
	std::string							 _host;
	std::vector<std::string> _server_names;

	bool				 _session_enable;
	std::string	 _session_name;
	std::string	 _session_path;
	unsigned int _session_timeout;

	std::map<std::string, LocationConfig> _locations;
	std::map<std::string, LocationConfig> _regex_locations;

public:
	ServerConfig();
	void setListen(unsigned int port);
	void setHost(const std::string& host);
	void addServerName(const std::string& name);
	void setServerNames(const std::vector<std::string>& names);
	void setSessionEnable(bool enable);
	void setSessionName(const std::string& name);
	void setSessionPath(const std::string& path);
	void setSessionTimeout(unsigned int timeout);
	void addLocation(const std::string& path, const LocationConfig& location);
	void addRegexLocation(const std::string&		regex,
												const LocationConfig& location);

	unsigned int																 getListen() const;
	const std::string&													 getHost() const;
	const std::vector<std::string>&							 getServerNames() const;
	bool																				 getSessionEnable() const;
	const std::string&													 getSessionName() const;
	const std::string&													 getSessionPath() const;
	unsigned int																 getSessionTimeout() const;
	const std::map<std::string, LocationConfig>& getLocation() const;
	const std::map<std::string, LocationConfig>& getRegexLocation() const;

	/*const LocationConfig* findLocation(const std::string& path) const;*/
	/*void applayGlobalConfig(const GlobalConfig& GlobalConfig);*/
	/*void applyToLocations();*/
};

#endif
