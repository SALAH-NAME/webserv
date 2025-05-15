#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "ConfigTypes.hpp"
#include <iostream>
#include <set>

void testConfigPrint()
{
	ConfigManager config;
	GlobalConfig& global_config = const_cast<GlobalConfig&>(config.getGlobalConfig());
	std::set<HttpMethod> allowed;
	allowed.insert(HTTP_GET);
	global_config.setAllowedMethods(allowed);


	std::vector<ServerConfig>& servers = const_cast<std::vector<ServerConfig>&>(config.getServers());
	ServerConfig server1;
	server1.setListen(80);
	server1.setHost("127.0.0.1");
	allowed.insert(HTTP_GET);
	allowed.insert(HTTP_POST);
	server1.setAllowedMethods(allowed);

	LocationConfig loc1;
	loc1.setPath("/api", false);
	loc1.setRoot("/var/www/example/api");
	loc1.setClientMaxBodySize("5M");
	loc1.setAutoindex(false);
	server1.addLocation("/api", loc1);

	servers.push_back(server1);

	ConfigPrinter printer(config);

	std::cout << std::endl;
	std::cout << "# Test Configuration" << std::endl;
	std::cout << std::endl;

	printer.print();
}

void testConfigPrinter()
{
    ConfigManager config;
    GlobalConfig& global_config = const_cast<GlobalConfig&>(config.getGlobalConfig());
    
    global_config.setConnectionTimeout(60);
    global_config.setCgiTimeout(30);
    global_config.setErrorLog("/var/log/webserv/error.log");
    global_config.setAccessLog("/var/log/webserv/access.log");
    global_config.setClientMaxBodySize("100M");
    global_config.setRoot("/var/www/default");
    
    std::vector<std::string> index_files;
    index_files.push_back("index.html");
    index_files.push_back("index.htm");
    index_files.push_back("default.html");
    global_config.setIndex(index_files);
    
    std::set<HttpMethod> global_allowed;
    global_allowed.insert(HTTP_GET);
    global_allowed.insert(HTTP_POST);
    global_config.setAllowedMethods(global_allowed);
    
    global_config.setAutoindex(false);
    global_config.setUploadStore("/var/www/uploads");
    
    global_config.setErrorPage(404, "/errors/404.html");
    global_config.setErrorPage(500, "/errors/500.html");
    global_config.setErrorPage(403, "/errors/403.html");
    
    std::vector<ServerConfig>& servers = const_cast<std::vector<ServerConfig>&>(config.getServers());
    
    ServerConfig server1;
    server1.setListen(80);
    server1.setHost("127.0.0.1");
    
    std::vector<std::string> server_names;
    server_names.push_back("example.com");
    server_names.push_back("www.example.com");
    server1.setServerNames(server_names);
    
    server1.setSessionEnable(true);
    server1.setSessionName("WEBSESSID");
    server1.setSessionPath("/");
    server1.setSessionTimeout(1800);
    
    server1.setRoot("/var/www/example");
    server1.setClientMaxBodySize("20M");
    
    std::set<HttpMethod> server_allowed;
    server_allowed.insert(HTTP_GET);
    server_allowed.insert(HTTP_POST);
    server_allowed.insert(HTTP_DELETE);
    server1.setAllowedMethods(server_allowed);
    
    server1.setAutoindex(true);
    server1.setUploadStore("/var/www/example/uploads");
    
    server1.setErrorPage(404, "/custom_errors/404.html");
    server1.setErrorPage(500, "/custom_errors/500.html");
    
    
    LocationConfig loc1;
    loc1.setPath("/api", false);
    loc1.setRoot("/var/www/example/api");
    loc1.setClientMaxBodySize("5M");
    loc1.setAutoindex(false);
    
    std::set<HttpMethod> loc1_allowed;
    loc1_allowed.insert(HTTP_GET);
    loc1_allowed.insert(HTTP_POST);
    loc1.setAllowedMethods(loc1_allowed);
    
    std::vector<std::string> loc1_index;
    loc1_index.push_back("api.html");
    loc1_index.push_back("index.json");
    loc1.setIndex(loc1_index);
    
    server1.addLocation("/api", loc1);
    
    LocationConfig loc2;
    loc2.setPath("/cgi-bin", false);
    loc2.setRoot("/var/www/example/cgi-bin");
    loc2.setCgiPass("/usr/bin/php-cgi");
    loc2.setCgitTimeout(15);
    loc2.setClientMaxBodySize("10M");
    loc2.setAutoindex(false);
    
    std::set<HttpMethod> loc2_allowed;
    loc2_allowed.insert(HTTP_GET);
    loc2_allowed.insert(HTTP_POST);
    loc2.setAllowedMethods(loc2_allowed);
    
    server1.addLocation("/cgi-bin", loc2);
    
    LocationConfig loc3;
    loc3.setPath("/old", false);
    loc3.setRedirect(301, "https://example.com/new");
    server1.addLocation("/old", loc3);
    
    LocationConfig loc4;
    loc4.setPath("\\.php$", true);
    loc4.setCgiPass("/usr/bin/php-cgi");
    loc4.setSessionTimeout(3600);
    server1.addRegexLocation("\\.php$", loc4);
    
    LocationConfig loc5;
    loc5.setPath("\\.py$", true);
    loc5.setCgiPass("/usr/bin/python3");
    loc5.setErrorPage(500, "/custom_errors/python_error.html");
    server1.addRegexLocation("\\.py$", loc5);
    
    servers.push_back(server1);
    
    ServerConfig server2;
    server2.setListen(80);
    server2.setHost("127.0.0.1");
    
    std::vector<std::string> server2_names;
    server2_names.push_back("another.com");
    server2_names.push_back("www.another.com");
    server2.setServerNames(server2_names);
    
    server2.setRoot("/var/www/another");
    server2.setAutoindex(false);
    
    LocationConfig server2_loc1;
    server2_loc1.setPath("/", false);
    server2_loc1.setRoot("/var/www/another/public");
    server2_loc1.setUploadStore("/var/www/another/uploads");
    server2.addLocation("/", server2_loc1);
    
    servers.push_back(server2);
    
    ServerConfig server3;
    server3.setListen(443);
    server3.setHost("127.0.0.1");
    
    std::vector<std::string> server3_names;
    server3_names.push_back("secure.example.com");
    server3.setServerNames(server3_names);
    
    server3.setRoot("/var/www/secure");
    
    LocationConfig server3_loc1;
    server3_loc1.setPath("/private", false);
    server3_loc1.setRoot("/var/www/secure/private");
    server3_loc1.setAutoindex(false);
    server3.addLocation("/private", server3_loc1);
    
    servers.push_back(server3);
    
    ConfigPrinter printer(config);
    
    std::cout << std::endl;
    std::cout << "# Comprehensive Configuration Test" << std::endl;
    std::cout << std::endl;
    
    printer.print();
}


void testZeroValuesConfigPrinter()
{
    ConfigManager config;
    
    GlobalConfig& global_config = const_cast<GlobalConfig&>(config.getGlobalConfig());
    global_config.setConnectionTimeout(0);
    global_config.setCgiTimeout(0);
    global_config.setErrorLog("");
    global_config.setAccessLog("");
    global_config.setClientMaxBodySize("0");
    global_config.setRoot("");
    
    std::vector<std::string> empty_index;
    global_config.setIndex(empty_index);
    
    std::set<HttpMethod> empty_methods;
    global_config.setAllowedMethods(empty_methods);
    
    global_config.setAutoindex(false);
    global_config.setUploadStore("");
    
    std::map<int, std::string> empty_error_pages;
    global_config.setErrorPages(empty_error_pages);
    
    std::vector<ServerConfig>& servers = const_cast<std::vector<ServerConfig>&>(config.getServers());
    
    ServerConfig zeroServer;
    zeroServer.setListen(0);
    zeroServer.setHost("");
    
    std::vector<std::string> empty_server_names;
    zeroServer.setServerNames(empty_server_names);
    
    zeroServer.setSessionEnable(false);
    zeroServer.setSessionName("");
    zeroServer.setSessionPath("");
    zeroServer.setSessionTimeout(0);
    
    zeroServer.setRoot("");
    zeroServer.setClientMaxBodySize("0");
    zeroServer.setAllowedMethods(empty_methods);
    zeroServer.setAutoindex(false);
    zeroServer.setUploadStore("");
    zeroServer.setErrorPages(empty_error_pages);
    zeroServer.setIndex(empty_index);
    
    LocationConfig zeroLocation;
    zeroLocation.setPath("", false);
    zeroLocation.setRoot("");
    zeroLocation.setClientMaxBodySize("0");
    zeroLocation.setAutoindex(false);
    zeroLocation.setAllowedMethods(empty_methods);
    zeroLocation.setIndex(empty_index);
    zeroLocation.setUploadStore("");
    zeroLocation.setErrorPages(empty_error_pages);
    
    zeroLocation.setCgiPass("");
    zeroLocation.setCgitTimeout(0);
    
    zeroLocation.setRedirect(0, "");
    
    zeroLocation.setSessionTimeout(0);
    
    zeroServer.addLocation("/zero", zeroLocation);
    
    LocationConfig zeroRegexLocation;
    zeroRegexLocation.setPath("", true);
    zeroRegexLocation.setRoot("");
    zeroRegexLocation.setCgiPass("");
    zeroRegexLocation.setCgitTimeout(0);
    zeroRegexLocation.setSessionTimeout(0);
    zeroRegexLocation.setClientMaxBodySize("0");
    zeroRegexLocation.setAutoindex(false);
    zeroRegexLocation.setAllowedMethods(empty_methods);
    zeroRegexLocation.setIndex(empty_index);
    zeroRegexLocation.setUploadStore("");
    zeroRegexLocation.setErrorPages(empty_error_pages);
    zeroRegexLocation.setRedirect(0, "");
    
    zeroServer.addRegexLocation("", zeroRegexLocation);
    
    servers.push_back(zeroServer);
    
    ConfigPrinter printer(config);
    
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "# Zero Values Configuration Test" << std::endl;
    std::cout << "# Testing how ConfigPrinter handles explicit zero/empty values" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    
    printer.print();
}
