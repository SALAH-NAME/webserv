#ifndef CONFIG_PRINTER_HPP
#define CONFIG_PRINTER_HPP

#include "ConfigManager.hpp"
#include <iostream>
#include <string>

class ConfigPrinter
{
private:
	const ConfigManager& _config_manager;

	void printGlobalConfig(std::ostream& out) const;
	void printServerConfig(std::ostream& out, const ServerConfig& server) const;
	void printBaseConfig(std::ostream& out, const BaseConfig& config,
											 int indent_level = 0) const;
	void printLocationConfig(std::ostream& out, const LocationConfig& location,
													 int indent_level = 0) const;

	void printIndent(std::ostream& out, int level) const;

	std::string methodSetToString(const std::set<HttpMethod>& methods) const;

public:
	ConfigPrinter(const ConfigManager& config_manager);
	~ConfigPrinter();

	void print(std::ostream& out = std::cout) const;

	std::string toString() const;
};

#endif
