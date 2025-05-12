#ifndef GLOBAL_CONFIG_HPP
#define GLOBAL_CONFIG_HPP

#include "BaseConfig.hpp"

class GlobalConfig : public BaseConfig
{
private:
	unsigned int _connection_timeout;
	unsigned int _cgi_timeout;
	std::string	 _error_log;
	std::string	 _access_log;

public:
	GlobalConfig();
	void setConnectionTimeout(unsigned int timeout);
	void setCgiTimeout(unsigned int timeout);
	void setErrorLog(const std::string& path);
	void setAccessLog(const std::string& path);

	unsigned int			 getConnectionTimeout() const;
	unsigned int			 getCgiTimeout() const;
	const std::string& getErrorLog() const;
	const std::string& getAccessLog() const;
};

#endif
