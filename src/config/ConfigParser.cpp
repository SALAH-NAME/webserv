#include "ConfigParser.hpp"
#include "BaseConfig.hpp"
#include "ConfigTokenizer.hpp"
#include "ConfigTypes.hpp"
#include "GlobalConfig.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

ConfigParser::ConfigParser(ConfigTokenizer& tokenizer) : _tokenizer(tokenizer)
{
}

ConfigParser::~ConfigParser() {}

bool ConfigParser::parse()
{
	try
	{
		parseConfig();
		applyInheritance();
		return true;
	}
	catch (const ParseError& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
}

const GlobalConfig& ConfigParser::getGlobalConfig() const
{
	return _global_config;
}

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
	return _servers;
}

void ConfigParser::parseConfig()
{
	parseGlobalDirectives();

	bool has_server = false;

	while (!_tokenizer.isEnd())
	{
		const ConfigTokenizer::TokenRecord& token = _tokenizer.front();

		if (token.type == CTT_SERVER)
		{
			has_server = true;
			_servers.push_back(parseServerBlock());
		}
		else if (token.type != CTT_COMMENT)
		{
			throw ParseError("Expected 'server' directive", token.line, token.column);
		}
		else
		{
			_tokenizer.pop();
		}
	}

	if (!has_server)
	{
		throw ParseError("Configuration must contain at least one server block", 0,
										 0);
	}
}

void ConfigParser::parseGlobalDirectives()
{
	while (!_tokenizer.isEnd())
	{
		const ConfigTokenizer::TokenRecord& token = _tokenizer.front();

		if (token.type == CTT_SERVER)
		{
			break;
		}
		else if (token.type == CTT_DIRECTIVE)
		{
			parseDirective(_global_config);
		}
		else
		{
			throw ParseError("Unknown Global Token: " + token.token, token.line,
											 token.column);
			/*_tokenizer.pop();*/
		}
	}
}

ServerConfig ConfigParser::parseServerBlock()
{
    ServerConfig server(_global_config.getConnectionTimeout());

	_tokenizer.pop();

	if (!expectToken(CTT_OPEN_BRACE, "Expected '{' after 'server'"))
	{
		throw ParseError("Expected '{' after 'server'", _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	server.inheritFrom(_global_config);

	while (!_tokenizer.isEnd())
	{
		const ConfigTokenizer::TokenRecord& token = _tokenizer.front();

		if (token.type == CTT_CLOSE_BRACE)
		{
			/*_tokenizer.pop();*/
			break;
		}
		else if (token.type == CTT_DIRECTIVE)
		{
			parseDirective(server, true);
		}
		else if (token.type == CTT_LOCATION)
		{
			_tokenizer.pop();

			std::string path;
			bool				is_regex = false;

			if (_tokenizer.front().type == CTT_TILDE)
			{
				is_regex = true;
				_tokenizer.pop();
				path = expectString("Expected regex pattern after '~'");

				if (!isValidRegexPath(path))
				{
					throw ParseError("Invalid regex path: " + path +
															 " (only .php and .py allowed)",
													 _tokenizer.front().line, _tokenizer.front().column);
				}
			}
			else
			{
				path = expectString("Expected path after 'location'");
			}

			LocationConfig location = parseLocationBlock();
			location.setPath(path, is_regex);

			location.inheritFrom(server);

			if (is_regex)
			{
				server.addRegexLocation(path, location);
			}
			else
			{
				server.addLocation(path, location);
			}
		}
		else
		{
			throw ParseError("Unknown Server Token: " + token.token, token.line,
											 token.column);
			/*_tokenizer.pop();*/
		}
	}

	if (!expectToken(CTT_CLOSE_BRACE, "Expected '}' at the end of server block"))
	{
		throw ParseError("Expected '}' at the end of server block",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	if (server.getHost().empty())
	{
		throw ParseError("Server block must specify a host directive",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	if (server.getListens().empty())
	{
		throw ParseError("Server block must specify a listen directive",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	return server;
}

LocationConfig ConfigParser::parseLocationBlock()
{
	LocationConfig location;

	if (!expectToken(CTT_OPEN_BRACE, "Expected '{' after location path"))
	{
		throw ParseError("Expected '{' after location path",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	while (!_tokenizer.isEnd())
	{
		const ConfigTokenizer::TokenRecord& token = _tokenizer.front();

		if (token.type == CTT_CLOSE_BRACE)
		{
			/*_tokenizer.pop();*/
			break;
		}
		else if (token.type == CTT_DIRECTIVE)
		{
			parseDirective(location, false, true);
		}
		else
		{
			throw ParseError("Unknown Location Token: " + token.token, token.line,
											 token.column);
			/*_tokenizer.pop();*/
		}
	}
	if (!expectToken(CTT_CLOSE_BRACE,
									 "Expected '}' at the end of location block"))
	{
		throw ParseError("Expected '}' at the end of location block",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	if (location.getRoot().empty() && !location.getRedirect().status_code)
	{
		throw ParseError("Location block must specify a root directive",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	return location;
}

void ConfigParser::parseDirective(BaseConfig& config, bool is_server,
																	bool is_location)
{
	const ConfigTokenizer::TokenRecord& token			= _tokenizer.front();
	std::string													directive = token.token;
	_tokenizer.pop();

	try
	{

		if (directive == "error_page")
		{
			parseErrorPage(config);
		}
		else if (directive == "client_max_body_size")
		{
			parseClientMaxBodySize(config);
		}
		else if (directive == "root")
		{
			parseRoot(config);
		}
		else if (directive == "index")
		{
			parseIndex(config);
		}
		else if (directive == "allowed_methods")
		{
			parseAllowedMethods(config);
		}
		else if (directive == "autoindex")
		{
			parseAutoindex(config);
		}
		else if (is_server)
		{
			ServerConfig& server = static_cast<ServerConfig&>(config);

			if (directive == "listen")
			{
				parseListen(server);
			}
			else if (directive == "host")
			{
				parseHost(server);
			}
			else if (directive == "server_name")
			{
				parseServerName(server);
			}
			else if (directive == "session_enable")
			{
				parseSessionEnable(server);
			}
			else if (directive == "session_name")
			{
				parseSessionName(server);
			}
			else if (directive == "session_path")
			{
				parseSessionPath(server);
			}
			else if (directive == "session_timeout")
			{
				parseSessionTimeout(server);
			}
			else if (directive == "connection_timeout")
			{
				parseConnectionTimeout(server);
			}
			else
			{
				throw ParseError("Unknown server directive: " + directive, token.line,
												 token.column);
			}
		}
		else if (is_location)
		{
			LocationConfig& location = static_cast<LocationConfig&>(config);

			if (directive == "return")
			{
				parseRedirect(location);
			}
			else if (directive == "cgi_pass")
			{
				parseCgiPass(location);
			}
			else if (directive == "cgi_timeout")
			{
				parseCgiTimeout(location);
			}
			else if (directive == "session_timeout")
			{
				parseSessionTimeout(location);
			}
			else
			{
				throw ParseError("Unknown location directive: " + directive, token.line,
												 token.column);
			}
		}
		else
		{
			GlobalConfig& global = static_cast<GlobalConfig&>(config);

			if (directive == "connection_timeout")
			{
				parseConnectionTimeout(global);
			}
			else if (directive == "cgi_timeout")
			{
				unsigned int timeout = expectNumber("Expected timeout value");
				global.setCgiTimeout(timeout);
				expectSemicolon("Expected semicolon after cgi_timeout directive");
			}
			else if (directive == "error_log")
			{
				parseErrorLog(global);
			}
			else if (directive == "access_log")
			{
				parseAccessLog(global);
			}
			else
			{
				throw ParseError("Unknown global directive: " + directive, token.line,
												 token.column);
			}
		}
	}
	catch (const ParseError& e)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw ParseError(e.what(), token.line, token.column);
	}
}

void ConfigParser::parseErrorPage(BaseConfig& config)
{
	std::map<int, std::string> error_pages;

	std::vector<int> error_codes;
	while (!_tokenizer.isEnd() && _tokenizer.front().type == CTT_NUMBER)
	{
		int code = expectNumber("Expected HTTP error code");
		if (code < 100 || code > 599)
		{
			throw ParseError("Invalid HTTP error code: " + to_string(code),
											 _tokenizer.front().line, _tokenizer.front().column);
		}
		error_codes.push_back(code);
	}

	std::string path = expectString("Expected path for error page");

	for (size_t i = 0; i < error_codes.size(); ++i)
	{
		config.setErrorPage(error_codes[i], path);
	}

	expectSemicolon("Expected semicolon after error_page directive");
}

void ConfigParser::parseClientMaxBodySize(BaseConfig& config)
{
	if (_tokenizer.front().type == CTT_SIZE ||
			_tokenizer.front().type == CTT_NUMBER)
	{
		std::string size = (_tokenizer.front().type == CTT_SIZE)
													 ? expectSize("Expected size value")
													 : to_string(expectNumber("Expected size value"));

		config.setClientMaxBodySize(size);
	}
	else
	{
		throw ParseError("Expected size value for client_max_body_size",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	expectSemicolon("Expected semicolon after client_max_body_size directive");
}

void ConfigParser::parseRoot(BaseConfig& config)
{
	std::string root = expectString("Expected path for root directive");
	if (!isValidPath(root))
	{
		throw ParseError("Invalid path: " + root, _tokenizer.front().line,
										 _tokenizer.front().column);
	}
	config.setRoot(root);
	expectSemicolon("Expected semicolon after root directive");
}

void ConfigParser::parseIndex(BaseConfig& config)
{
	std::vector<std::string> index_files;

	while (!_tokenizer.isEnd() && _tokenizer.front().type == CTT_STRING)
	{
		index_files.push_back(
				expectString("Expected filename for index directive"));
	}

	if (index_files.empty())
	{
		throw ParseError("Expected at least one index file",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	config.setIndex(index_files);
	expectSemicolon("Expected semicolon after index directive");
}

void ConfigParser::parseAllowedMethods(BaseConfig& config)
{
	std::set<HttpMethod> methods;

	while (!_tokenizer.isEnd() && _tokenizer.front().type == CTT_STRING)
	{
		std::string method_str = expectString("Expected HTTP method");

		if (!isValidHttpMethod(method_str))
		{
			throw ParseError("Invalid HTTP method: " + method_str,
											 _tokenizer.front().line, _tokenizer.front().column);
		}

		methods.insert(stringToHttpMethod(method_str));
	}

	if (methods.empty())
	{
		throw ParseError("Expected at least one HTTP method",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	config.setAllowedMethods(methods);
	expectSemicolon("Expected semicolon after allowed_methods directive");
}

void ConfigParser::parseAutoindex(BaseConfig& config)
{
	std::string value =
			expectString("Expected 'on' or 'off' for autoindex directive");

	if (value == "on")
	{
		config.setAutoindex(true);
	}
	else if (value == "off")
	{
		config.setAutoindex(false);
	}
	else
	{
		throw ParseError("Invalid autoindex value: " + value +
												 " (expected 'on' or 'off')",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	expectSemicolon("Expected semicolon after autoindex directive");
}

void ConfigParser::parseListen(ServerConfig& server)
{
	int port = expectNumber("Expected port number for listen directive");

	if (!isValidPort(port))
	{
		throw ParseError("Invalid port number: " + to_string(port),
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	server.addListen(port);
	expectSemicolon("Expected semicolon after listen directive");
}

void ConfigParser::parseHost(ServerConfig& server)
{
	std::string host = expectString("Expected IP address for host directive");

	if (!isValidIP(host))
	{
		throw ParseError("Invalid IP address: " + host, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	server.setHost(host);
	expectSemicolon("Expected semicolon after host directive");
}

void ConfigParser::parseServerName(ServerConfig& server)
{
	std::vector<std::string> names;

	while (!_tokenizer.isEnd() && _tokenizer.front().type == CTT_STRING)
	{
		std::string name =
				expectString("Expected domain name for server_name directive");

		if (!isValidDomain(name))
		{
			throw ParseError("Invalid domain name: " + name, _tokenizer.front().line,
											 _tokenizer.front().column);
		}

		names.push_back(name);
	}

	if (names.empty())
	{
		throw ParseError("Expected at least one server name",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	server.setServerNames(names);
	expectSemicolon("Expected semicolon after server_name directive");
}

void ConfigParser::parseSessionEnable(ServerConfig& server)
{
	std::string value =
			expectString("Expected 'on' or 'off' for session_enable directive");

	if (value == "on")
	{
		server.setSessionEnable(true);
	}
	else if (value == "off")
	{
		server.setSessionEnable(false);
	}
	else
	{
		throw ParseError("Invalid session_enable value: " + value +
												 " (expected 'on' or 'off')",
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	expectSemicolon("Expected semicolon after session_enable directive");
}

void ConfigParser::parseSessionName(ServerConfig& server)
{
	std::string name = expectString("Expected name for session_name directive");
	server.setSessionName(name);
	expectSemicolon("Expected semicolon after session_name directive");
}

void ConfigParser::parseSessionPath(ServerConfig& server)
{
	std::string path = expectString("Expected path for session_path directive");

	if (!isValidPath(path))
	{
		throw ParseError("Invalid path: " + path, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	server.setSessionPath(path);
	expectSemicolon("Expected semicolon after session_path directive");
}

void ConfigParser::parseSessionTimeout(ServerConfig& server)
{
	int timeout =
			expectNumber("Expected timeout value for session_timeout directive");

	if (!isValidTimeout(timeout))
	{
		throw ParseError("Invalid timeout value: " + to_string(timeout),
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	server.setSessionTimeout(timeout);
	expectSemicolon("Expected semicolon after session_timeout directive");
}

void ConfigParser::parseSessionTimeout(LocationConfig& location)
{
	int timeout =
			expectNumber("Expected timeout value for session_timeout directive");

	if (!isValidTimeout(timeout))
	{
		throw ParseError("Invalid timeout value: " + to_string(timeout),
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	location.setSessionTimeout(timeout);
	expectSemicolon("Expected semicolon after session_timeout directive");
}

void ConfigParser::parseRedirect(LocationConfig& location)
{
	int code = expectNumber("Expected status code for return directive");

	if (code < 300 || code > 308)
	{
		throw ParseError("Invalid redirect status code: " + to_string(code),
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	std::string url = expectString("Expected URL for return directive");
	location.setRedirect(code, url);

	expectSemicolon("Expected semicolon after return directive");
}

void ConfigParser::parseCgiPass(LocationConfig& location)
{
	std::string path = expectString("Expected path for cgi_pass directive");

	if (!isValidPath(path))
	{
		throw ParseError("Invalid path: " + path, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	location.setCgiPass(path);
	expectSemicolon("Expected semicolon after cgi_pass directive");
}

void ConfigParser::parseCgiTimeout(LocationConfig& location)
{
	int timeout =
			expectNumber("Expected timeout value for cgi_timeout directive");

	if (!isValidTimeout(timeout))
	{
		throw ParseError("Invalid timeout value: " + to_string(timeout),
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	location.setCgitTimeout(timeout);
	expectSemicolon("Expected semicolon after cgi_timeout directive");
}

void ConfigParser::parseConnectionTimeout(GlobalConfig& global)
{
	int timeout =
			expectNumber("Expected timeout value for connection_timeout directive");

	if (!isValidTimeout(timeout))
	{
		throw ParseError("Invalid timeout value: " + to_string(timeout),
										 _tokenizer.front().line, _tokenizer.front().column);
	}

	global.setConnectionTimeout(timeout);
	expectSemicolon("Expected semicolon after connection_timeout directive");
}

void ConfigParser::parseConnectionTimeout(ServerConfig& server) {
    int timeout = expectNumber("Expected timeout value for connection_timeout directive");
    if (!isValidTimeout(timeout)) {
        throw ParseError("Invalid timeout value: " + to_string(timeout), _tokenizer.front().line, _tokenizer.front().column);
    }
    server.setConnectionTimeout(timeout);
    expectSemicolon("Expected semicolon after connection_timeout directive");
}

void ConfigParser::parseErrorLog(GlobalConfig& global)
{
	std::string path = expectString("Expected path for error_log directive");

	if (!isValidPath(path))
	{
		throw ParseError("Invalid path: " + path, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	global.setErrorLog(path);
	expectSemicolon("Expected semicolon after error_log directive");
}

void ConfigParser::parseAccessLog(GlobalConfig& global)
{
	std::string path = expectString("Expected path for access_log directive");

	if (!isValidPath(path))
	{
		throw ParseError("Invalid path: " + path, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	global.setAccessLog(path);
	expectSemicolon("Expected semicolon after access_log directive");
}

bool ConfigParser::expectToken(ConfigTokenType		expected_type,
															 const std::string& error_message)
{
	if (_tokenizer.isEnd() || _tokenizer.front().type != expected_type)
	{
		throw ParseError(error_message, _tokenizer.front().line,
										 _tokenizer.front().column);
		return false;
	}

	_tokenizer.pop();
	return true;
}

std::string ConfigParser::expectString(const std::string& error_message)
{
	if (_tokenizer.isEnd() || _tokenizer.front().type != CTT_STRING)
	{
		throw ParseError(error_message, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	std::string value = _tokenizer.front().token;
	_tokenizer.pop();
	return value;
}

int ConfigParser::expectNumber(const std::string& error_message)
{
	if (_tokenizer.isEnd() || _tokenizer.front().type != CTT_NUMBER)
	{
		throw ParseError(error_message, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	int value = atoi(_tokenizer.front().token.c_str());
	_tokenizer.pop();
	return value;
}

std::string ConfigParser::expectSize(const std::string& error_message)
{
	if (_tokenizer.isEnd() || _tokenizer.front().type != CTT_SIZE)
	{
		throw ParseError(error_message, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	std::string value = _tokenizer.front().token;
	_tokenizer.pop();
	return value;
}

void ConfigParser::expectSemicolon(const std::string& error_message)
{
	if (_tokenizer.isEnd() || _tokenizer.front().type != CTT_SEMICOLON)
	{
		throw ParseError(error_message, _tokenizer.front().line,
										 _tokenizer.front().column);
	}

	_tokenizer.pop();
}

bool ConfigParser::isValidIP(const std::string& ip)
{
	std::istringstream iss(ip);
	std::string				 octet;
	int								 count = 0;

	while (std::getline(iss, octet, '.'))
	{
		if (octet.empty() || (octet.size() > 1 && octet[0] == '0'))
		{
			return false;
		}

		for (size_t i = 0; i < octet.size(); ++i)
		{
			if (!isdigit(octet[i]))
			{
				return false;
			}
		}

		int num = atoi(octet.c_str());

		if (num < 0 || num > 255)
		{
			return false;
		}

		count++;
	}

	return count == 4;
}

bool ConfigParser::isValidPort(int port) { return port > 0 && port <= 65535; }

bool ConfigParser::isValidPath(const std::string& path)
{
	return !path.empty() && (path[0] == '/' || (path.length() >= 2 && path.substr(0, 2) == "./"));
}

bool ConfigParser::isValidTimeout(int timeout)
{
	return timeout >= 1 && timeout <= 86400;
}

bool ConfigParser::isValidDomain(const std::string& domain)
{
	if (domain.empty())
		return true;

	if (domain.length() > 255)
		return false;

	size_t start	 = 0;
	size_t dot_pos = domain.find('.');

	while (start < domain.length())
	{
		size_t length = (dot_pos == std::string::npos) ? domain.length() - start
																									 : dot_pos - start;

		if (length == 0 || length > 63)
			return false;

		for (size_t i = start; i < start + length; ++i)
		{
			char c = domain[i];
			if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
						(c >= '0' && c <= '9') || c == '-'))
			{
				return false;
			}
		}

		if (domain[start] == '-' ||
				(dot_pos != std::string::npos && domain[dot_pos - 1] == '-'))
		{
			return false;
		}

		if (dot_pos == std::string::npos)
			break;

		start		= dot_pos + 1;
		dot_pos = domain.find('.', start);
	}

	return true;
}

bool ConfigParser::isValidHttpMethod(const std::string& method)
{
	std::string upper_method = method;
	for (size_t i = 0; i < upper_method.size(); ++i)
		upper_method[i] = toupper(upper_method[i]);

	return upper_method == "GET" || upper_method == "POST" ||
				 upper_method == "DELETE";
}

bool ConfigParser::isValidRegexPath(const std::string& path)
{
	return path == ".php" || path == ".py" || path == ".sh";
}

void ConfigParser::applyInheritance()
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		ServerConfig& server = _servers[i];
		server.inheritFrom(_global_config);

		const std::map<std::string, LocationConfig>& locations =
				server.getLocations();
		for (std::map<std::string, LocationConfig>::const_iterator loc_it =
						 locations.begin();
				 loc_it != locations.end(); ++loc_it)
		{
			std::string		 path			= loc_it->first;
			LocationConfig location = loc_it->second;
			location.inheritFrom(server);

			server.addLocation(path, location);
		}

		const std::map<std::string, LocationConfig>& regex_locations =
				server.getRegexLocation();
		for (std::map<std::string, LocationConfig>::const_iterator regex_it =
						 regex_locations.begin();
				 regex_it != regex_locations.end(); ++regex_it)
		{
			std::string		 regex_path = regex_it->first;
			LocationConfig location		= regex_it->second;
			location.inheritFrom(server);

			server.addRegexLocation(regex_path, location);
		}
	}
}
