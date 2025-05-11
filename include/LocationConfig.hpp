#ifndef LOCATION_CONFIG_HPP
#define LOCATION_CONFIG_HPP

#include "BaseConfig.hpp"
#include "ConfigTypes.hpp"

class LocationConfig : public BaseConfig
{
private:
	RedirectInfo _redirect;
	std::string	 _cgi_pass;
	unsigned int _cgi_timeout;
	std::string	 _path;
	bool				 _is_regex;

public:
	LocationConfig();
	void setRedirect(int status_code, const std::string &url);
	void setCgiPass(const std::string &path);
	void setCgitTimeout(unsigned int timeout);
	void setPath(const std::string &path, bool is_regex = false);

	const RedirectInfo &getRedirect() const;
	const std::string	 &getCgiPass() const;
	unsigned int				getCgiTimeout() const;
	const std::string	 &getPath() const;
	bool								isRegex() const;
	bool								isCgi() const;
	bool								hasRedirect() const;
	void								inheritFrom(const BaseConfig &parent);
};

#endif
