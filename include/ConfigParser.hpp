#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "ConfigTokenizer.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class ConfigParser
{
private:
	ConfigTokenizer&					_tokenizer;
	GlobalConfig							_global_config;
	std::vector<ServerConfig> _servers;

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
	void parseUploadStore(BaseConfig& config);

	void parseListen(ServerConfig& server);
	void parseHost(ServerConfig& server);
	void parseServerName(ServerConfig& server);
	void parseSessionEnable(ServerConfig& server);
	void parseSessionName(ServerConfig& server);
	void parseSessionPath(ServerConfig& server);
	void parseSessionTimeout(ServerConfig& server);
	void parseConnectionTimeout(ServerConfig& server);

	void parseRedirect(LocationConfig& location);
	void parseCgiPass(LocationConfig& location);
	void parseCgiTimeout(LocationConfig& location);
	void parseSessionTimeout(LocationConfig& location);

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

	void applyInheritance();

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
