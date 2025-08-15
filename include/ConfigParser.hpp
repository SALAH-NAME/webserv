#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "ConfigTokenizer.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/types.h>

class ConfigParser
{
private:
	ConfigTokenizer&					_tokenizer;
	GlobalConfig							_global_config;
	std::vector<ServerConfig> _servers;

	inline bool s_isDir(mode_t mode);
	inline bool s_isReg(mode_t mode);
	void					 parseConfig();
	void					 parseGlobalDirectives();
	ServerConfig	 parseServerBlock();
	LocationConfig parseLocationBlock();

	void parseDirective(BaseConfig& config, bool is_server = false,
											bool is_location = false);

	void parseErrorPage(BaseConfig& config);
	void parseClientMaxBodySize(BaseConfig& config);
	void parseRoot(BaseConfig& config);
	void parseIndex(BaseConfig& config);
	void parseAllowedMethods(BaseConfig& config);
	void parseAutoindex(BaseConfig& config);

	void parseListen(ServerConfig& server);
	void parseHost(ServerConfig& server);
	void parseServerName(ServerConfig& server);
	void parseConnectionTimeout(ServerConfig& server);

	void parseRedirect(LocationConfig& location);
	void parseCgiPass(LocationConfig& location);
	void parseCgiTimeout(LocationConfig& location);

	void parseConnectionTimeout(GlobalConfig& global);
	void parseErrorLog(GlobalConfig& global);
	void parseAccessLog(GlobalConfig& global);

	bool				expectToken(ConfigTokenType		 expected_type,
													const std::string& error_message);
	std::string expectString(const std::string& error_message);
	int					expectNumber(const std::string& error_message);
	std::string expectSize(const std::string& error_message);
	void				expectSemicolon(const std::string& error_message);

	bool isValidIP(const std::string& ip);
	bool isValidPort(int port);
	bool isValidPath(const std::string& path);
	bool isValidTimeout(int timeout);
	bool isValidDomain(const std::string& domain);
	bool isValidHttpMethod(const std::string& method);
	bool isValidRegexPath(const std::string& path);
	bool isExternalUrl(const std::string& url);
	bool isValidUrl(const std::string& url);

	bool isPathAccessible(const std::string& path);
	bool isExecutableAccessible(const std::string& path);
	bool isDirectoryAccessible(const std::string& path);
	void validateRootPath(const std::string& path);
	void validateErrorPagePath(const std::string& path);
	void validateCgiPassPath(const std::string& path);

	void applyInheritance();
	void validateCircularRedirects();
	
	bool serverHasRedirects(const ServerConfig& server);
	void checkDuplicateLocationPath(const ServerConfig& server, const std::string& path, bool is_regex);
	void buildRedirectMap(const ServerConfig& server, std::map<std::string, std::string>& redirect_map);
	bool detectCircularRedirect(const std::string& start_path, const std::map<std::string, std::string>& redirect_map, std::set<std::string>& global_visited);
	std::string buildCircularRedirectErrorMessage(const std::vector<std::string>& path_stack, const std::string& current_path);

public:
	class ParseError : public std::exception
	{
	private:
		std::string _message;

	public:
		ParseError(const std::string& message, size_t line, size_t column)
		{
			std::ostringstream oss;
			oss << "Parse error at line " << line << ", column " << column << ": "
					<< message;
			_message = oss.str();
		}

		virtual ~ParseError() throw() {}

		virtual const char* what() const throw() { return _message.c_str(); }
	};

	ConfigParser(ConfigTokenizer& tokenizer);
	~ConfigParser();

	bool parse();

	const GlobalConfig&							 getGlobalConfig() const;
	const std::vector<ServerConfig>& getServers() const;
};

#endif
