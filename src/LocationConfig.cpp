#include "LocationConfig.hpp"
#include "BaseConfig.hpp"
#include "ConfigTypes.hpp"
#include "DefaultConfig.hpp"
#include <string>

LocationConfig::LocationConfig()
		: BaseConfig(), _cgi_timeout(DefaultConfig::CGI_TIMEOUT),
			_is_regex(DefaultConfig::IS_REGEX)
{
}

void LocationConfig::setRedirect(int status_code, const std::string &url)
{
	_redirect = RedirectInfo(status_code, url);
}

void LocationConfig::setCgiPass(const std::string &path) { _cgi_pass = path; }

void LocationConfig::setCgitTimeout(unsigned int timeout)
{
	_cgi_timeout = timeout;
}

void LocationConfig::setPath(const std::string &path, bool is_regex)
{
	_path			= path;
	_is_regex = is_regex;
}

const RedirectInfo &LocationConfig::getRedirect() const { return _redirect; }

const std::string &LocationConfig::getCgiPass() const { return _cgi_pass; }

unsigned int LocationConfig::getCgiTimeout() const { return _cgi_timeout; }

const std::string &LocationConfig::getPath() const { return _path; }

bool LocationConfig::isRegex() const { return _is_regex; }

bool LocationConfig::isCgi() const { return !_cgi_pass.empty(); }

bool LocationConfig::hasRedirect() const { return _redirect.isValid(); }

void LocationConfig::inheritFrom(const BaseConfig &parent)
{
	BaseConfig::inheritFrom(parent);
	if (_cgi_timeout == 0)
	{
		const LocationConfig *loc = dynamic_cast<const LocationConfig *>(&parent);
		if (loc)
			_cgi_timeout = loc->getCgiTimeout();
	}
}
