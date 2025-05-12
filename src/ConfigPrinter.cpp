#include "ConfigPrinter.hpp"
#include "ConfigManager.hpp"
#include "GlobalConfig.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include <ostream>
#include <sstream>

ConfigPrinter::ConfigPrinter(const ConfigManager& config_manager)
		: _config_manager(config_manager)
{
}

ConfigPrinter::~ConfigPrinter() {}

void ConfigPrinter::print(std::ostream& out) const
{
	const std::vector<ServerConfig>& servers = _config_manager.getServers();

	printGlobalConfig(out);

	out << std::endl;

	for (size_t i = 0; i < servers.size(); ++i)
	{
		printServerConfig(out, servers[i]);
		if (i < servers.size() - 1)
			out << std::endl;
	}
}

std::string ConfigPrinter::toString() const
{
	std::ostringstream ss;
	print(ss);
	return ss.str();
}

void ConfigPrinter::printGlobalConfig(std::ostream& out) const
{
	const GlobalConfig& global = _config_manager.getGlobalConfig();

	out << "# Global Configuration" << std::endl;
	out << "connection_timeout " << global.getConnectionTimeout() << ";"
			<< std::endl;
	out << "cgi_timeout " << global.getCgiTimeout() << ";" << std::endl;
	if (!global.getErrorLog().empty())
		out << "error_log " << global.getErrorLog() << ";" << std::endl;
	if (!global.getAccessLog().empty())
		out << "access_log " << global.getAccessLog() << ";" << std::endl;
	printBaseConfig(out, global);
}

void ConfigPrinter::printServerConfig(std::ostream&				out,
																			const ServerConfig& server) const
{
	out << "server {" << std::endl;
	out << "    listen " << server.getListen() << ";" << std::endl;
	out << "    host " << server.getHost() << ";" << std::endl;

	const std::vector<std::string>& server_names = server.getServerNames();
	if (!server_names.empty())
	{
		out << "    server_name";
		for (size_t i = 0; i < server_names.size(); ++i)
			out << " " << server_names[i];
		out << ";" << std::endl;
	}

	printBaseConfig(out, server, 1);

	std::map<std::string, LocationConfig>::const_iterator it;

	const std::map<std::string, LocationConfig>& locations =
			server.getLocations();
	for (it = locations.begin(); it != locations.end(); ++it)
	{
		out << std::endl;
		out << "    location " << it->first << " {" << std::endl;
		printLocationConfig(out, it->second, 2);
		out << "    }" << std::endl;
	}

	const std::map<std::string, LocationConfig>& regex_locations =
			server.getRegexLocation();
	for (it = regex_locations.begin(); it != regex_locations.end(); ++it)
	{
		out << std::endl;
		out << "    location ~ " << it->first << " {" << std::endl;
		printLocationConfig(out, it->second, 2);
		out << "    }" << std::endl;
	}
	out << "}" << std::endl;
}

void ConfigPrinter::printBaseConfig(std::ostream& out, const BaseConfig& config,
																		int indent_level) const
{
	printIndent(out, indent_level);
	out << "client_max_body_size " << config.getClientMaxBodySize() << ";"
			<< std::endl;

	if (!config.getRoot().empty())
	{
		printIndent(out, indent_level);
		out << "root " << config.getRoot() << ";" << std::endl;
	}

	const std::vector<std::string>& index_files = config.getIndex();
	if (!index_files.empty())
	{
		printIndent(out, indent_level);
		out << "index";
		for (size_t i = 0; i < index_files.size(); ++i)
		{
			out << " " << index_files[i];
		}
		out << ";" << std::endl;
	}

	printIndent(out, indent_level);
	out << "allow_methods " << methodSetToString(config.getAllowedMethods())
			<< ";" << std::endl;

	printIndent(out, indent_level);
	out << "autoindex " << (config.getAutoindex() ? "on" : "off") << ";"
			<< std::endl;

	if (!config.getUploadStore().empty())
	{
		printIndent(out, indent_level);
		out << "upload_store " << config.getUploadStore() << ";" << std::endl;
	}

	const std::map<int, std::string>& error_pages = config.getErrorPages();
	std::map<int, std::string>::const_iterator it;
	for (it = error_pages.begin(); it != error_pages.end(); ++it)
	{
		printIndent(out, indent_level);
		out << "error_page " << it->first << " " << it->second << ";" << std::endl;
	}
}

void ConfigPrinter::printLocationConfig(std::ostream&					out,
																				const LocationConfig& location,
																				int indent_level) const
{
	const RedirectInfo& redirect = location.getRedirect();
	if (location.hasRedirect())
	{
		printIndent(out, indent_level);
		out << "redirect " << redirect.status_code << " " << redirect.url << ";"
				<< std::endl;
	}

	if (location.isCgi())
	{
		printIndent(out, indent_level);
		out << "cgi_pass " << location.getCgiPass() << ";" << std::endl;

		if (location.getCgiTimeout() > 0)
		{
			printIndent(out, indent_level);
			out << "cgi_timeout " << location.getCgiTimeout() << ";" << std::endl;
		}
	}

	printBaseConfig(out, location, indent_level);
}

void ConfigPrinter::printIndent(std::ostream& out, int level) const
{
	for (int i = 0; i < level; ++i)
		out << "    ";
}

std::string
ConfigPrinter::methodSetToString(const std::set<HttpMethod>& methods) const
{
	std::string													 result;
	std::set<HttpMethod>::const_iterator it;

	for (it = methods.begin(); it != methods.end(); ++it)
	{
		if (it != methods.begin())
			result += " ";
		result += httpMethodToString(*it);
	}

	return result;
}
