#include "Request.hpp"

Request::Request(void) {}

Request::~Request(void) {}

std::string	Request::getHost(void) {
	return _host;
}

std::string	Request::getMethod(void) {
	return _method;
}

std::string	Request::getPath(void) {
	return _path;
}

std::string	Request::getVersion(void) {
	return _version;
}

std::string	Request::getConnection(void) {
	return _connection;
}

void	Request::setHost(std::string host) {
	_host = host;
}

void	Request::setMethod(std::string method) {
	_method = method;
}

void	Request::setPath(std::string path) {
	_path = path;
}

void	Request::setVersion(std::string version) {
	_version = version;
}

void	Request::setConnection(std::string connection) {
	_connection = connection;
}

void    Request::set_up(std::string host, std::string method, std::string path, std::string version, std::string connection) {
	setHost(host);
	setMethod(method);
	setPath(path);
	setVersion(version);
	setConnection(connection);
}

void    Request::printer(void) {

	std::cout << "---------- REQUEST: ------------\n";
	std::cout << "Host: " << _host << "\n";
	std::cout << "Method: " << _method << "\n";
	std::cout << "Path: " << _path << "\n";
	std::cout << "Version: " << _version << "\n";
	std::cout << "Connection: " << _connection << "\n";
}
