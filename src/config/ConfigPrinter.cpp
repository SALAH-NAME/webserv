#include "ConfigPrinter.hpp"
#include "ConfigManager.hpp"
#include "GlobalConfig.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include <ostream>
#include <sstream>

#include "ConfigPrinter.hpp"
#include <sstream>
#include <vector>

ConfigPrinter::ConfigPrinter(const ConfigManager& config_manager)
		: _config_manager(config_manager)
{
}

ConfigPrinter::~ConfigPrinter() {}

void ConfigPrinter::print(std::ostream& out) const
{
	printGlobalConfig(out);

	const std::vector<ServerConfig>& servers = _config_manager.getServers();
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (i > 0)
		{
			out << "\n";
		}
		printServerConfig(out, servers[i]);
	}
}

std::string ConfigPrinter::toString() const
{
	std::ostringstream oss;
	print(oss);
	return oss.str();
}

void ConfigPrinter::printGlobalConfig(std::ostream& out) const
{
	const GlobalConfig& global = _config_manager.getGlobalConfig();

	out << "# Global Configuration\n";

	out << "connection_timeout " << global.getConnectionTimeout() << ";\n";
	out << "cgi_timeout " << global.getCgiTimeout() << ";\n";

	if (!global.getErrorLog().empty())
	{
		out << "error_log " << global.getErrorLog() << ";\n";
	}

	if (!global.getAccessLog().empty())
	{
		out << "access_log " << global.getAccessLog() << ";\n";
	}

	printBaseConfig(out, global);

	out << "\n";
}

void ConfigPrinter::printServerConfig(std::ostream&				out,
																			const ServerConfig& server) const
{
	out << "# Server Configuration\n";
	out << "server {\n";

	std::vector<unsigned int> ports = server.getListens();
	printIndent(out, 1);
	for (unsigned int i = 0 ; i < ports.size(); ++i)
	{
		out << "listen " << ports[i] << ";\n";
		printIndent(out, 1);
	}
	out << "host " << server.getHost() << ";\n";

	const std::vector<std::string>& server_names = server.getServerNames();
	if (!server_names.empty())
	{
		printIndent(out, 1);
		out << "server_name";
		for (std::vector<std::string>::const_iterator it = server_names.begin();
				 it != server_names.end(); ++it)
		{
			out << " " << *it;
		}
		out << ";\n";
	}

	if (server.getConnectionTimeout())
	{
		printIndent(out, 1);
		out << "connection_timeout ";
		out << server.getConnectionTimeout() << ";\n";
	}

	printBaseConfig(out, server, 1);

	const std::map<std::string, LocationConfig>& locations =
			server.getLocations();
	for (std::map<std::string, LocationConfig>::const_iterator it =
					 locations.begin();
			 it != locations.end(); ++it)
	{
		out << "\n";
		printIndent(out, 1);
		out << "location " << it->first << " {\n";
		printLocationConfig(out, it->second, 2);
		printIndent(out, 1);
		out << "}\n";
	}

	const std::map<std::string, LocationConfig>& regex_locations =
			server.getRegexLocation();
	for (std::map<std::string, LocationConfig>::const_iterator it =
					 regex_locations.begin();
			 it != regex_locations.end(); ++it)
	{
		out << "\n";
		printIndent(out, 1);
		out << "location ~ " << it->first << " {\n";
		printLocationConfig(out, it->second, 2);
		printIndent(out, 1);
		out << "}\n";
	}

	out << "}\n";
}

void ConfigPrinter::printBaseConfig(std::ostream& out, const BaseConfig& config,
																		int indent_level) const
{
	const std::map<int, std::string>& error_pages = config.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = error_pages.begin();
			 it != error_pages.end(); ++it)
	{
		printIndent(out, indent_level);
		out << "error_page " << it->first << " " << it->second << ";\n";
	}

	if (config.getClientMaxBodySize() > 0)
	{
		printIndent(out, indent_level);
		out << "client_max_body_size " << config.getClientMaxBodySize() << ";\n";
	}

	if (!config.getRoot().empty())
	{
		printIndent(out, indent_level);
		out << "root " << config.getRoot() << ";\n";
	}

	const std::vector<std::string>& index_files = config.getIndex();
	if (!index_files.empty())
	{
		printIndent(out, indent_level);
		out << "index";
		for (std::vector<std::string>::const_iterator it = index_files.begin();
				 it != index_files.end(); ++it)
		{
			out << " " << *it;
		}
		out << ";\n";
	}

	const std::set<HttpMethod>& allowed_methods = config.getAllowedMethods();
	if (!allowed_methods.empty())
	{
		printIndent(out, indent_level);
		out << "allowed_methods " << methodSetToString(allowed_methods) << ";\n";
	}

	printIndent(out, indent_level);
	out << "autoindex " << (config.getAutoindex() ? "on" : "off") << ";\n";
}

void ConfigPrinter::printLocationConfig(std::ostream&					out,
																				const LocationConfig& location,
																				int indent_level) const
{
	printBaseConfig(out, location, indent_level);

	const RedirectInfo& redirect = location.getRedirect();
	if (redirect.isValid())
	{
		printIndent(out, indent_level);
		out << "return " << redirect.status_code << " " << redirect.url << ";\n";
	}

	if (!location.getCgiPass().empty())
	{
		printIndent(out, indent_level);
		out << "cgi_pass " << location.getCgiPass() << ";\n";

		if (location.getCgiTimeout() > 0)
		{
			printIndent(out, indent_level);
			out << "cgi_timeout " << location.getCgiTimeout() << ";\n";
		}
	}
}

void ConfigPrinter::printIndent(std::ostream& out, int level) const
{
	for (int i = 0; i < level; ++i)
	{
		out << "    ";
	}
}

std::string
ConfigPrinter::methodSetToString(const std::set<HttpMethod>& methods) const
{
	std::string result;
	for (std::set<HttpMethod>::const_iterator it = methods.begin();
			 it != methods.end(); ++it)
	{
		if (it != methods.begin())
		{
			result += " ";
		}
		result += httpMethodToString(*it);
	}
	return result;
}
