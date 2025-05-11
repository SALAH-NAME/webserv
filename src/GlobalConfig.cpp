#include "GlobalConfig.hpp"
#include "BaseConfig.hpp"
#include "DefaultConfig.hpp"

GlobalConfig::GlobalConfig()
		: BaseConfig(), _connection_timeout(DefaultConfig::CONNECTION_TIMEOUT),
			_cgi_timeout(DefaultConfig::CGI_TIMEOUT)
{
}

void GlobalConfig::setConnectionTimeout(unsigned int timeout)
{
	_connection_timeout = timeout;
}

void GlobalConfig::setCgiTimeout(unsigned int timeout)
{
	_cgi_timeout = timeout;
}

void GlobalConfig::setErrorLog(const std::string &path) { _error_log = path; }

void GlobalConfig::setAccessLog(const std::string &path) { _access_log = path; }

unsigned int GlobalConfig::getConnectionTimeout() const
{
	return _connection_timeout;
}

unsigned int GlobalConfig::getCgiTimeout() const { return _cgi_timeout; }

const std::string &GlobalConfig::getErrorLog() const { return _error_log; }

const std::string &GlobalConfig::getAccessLog() const { return _access_log; }
