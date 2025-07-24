#ifndef DEFAULT_CONFIG_HPP
#define DEFAULT_CONFIG_HPP

#include "ConfigTypes.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

class SizeValue;

namespace DefaultConfig
{
// BASE
const std::map<int, std::string> ERROR_PAGES;
const std::string								 CLIENT_MAX_BODY_SIZE("1M");
const std::string								 ROOT("/var/www/html");
const std::vector<std::string>	 INDEX(1, "index.html");
const std::set<HttpMethod>			 ALLOWED_METHODS;
const int												 AUTOINDEX(false);
const std::string								 UPLOAD_STORE("");

// GLOBAL
const unsigned int CONNECTION_TIMEOUT(60);
const unsigned int CGI_TIMEOUT(60);
const std::string	 ERROR_LOG;
const std::string	 ACCESS_LOG;

// SERVER
const std::vector<unsigned int>	LISTEN(0);
const std::string								HOST("");
const std::vector<std::string>	SERVER_NAMES(1, "");
const int												SESION_ENABLE(false);
const std::string								SESSION_NAME;
const std::string								SESSION_PATH;
const unsigned int							SESSION_TIMEOUT(1800);

// LOCATION
const RedirectInfo REDIRECT;
const std::string	 CGI_PASS("");
const unsigned int CGIT_TIMEOUT(60);
const int					 IS_REGEX(false);
} // namespace DefaultConfig

#endif // !DEFAULT_CONFIG_HPP
