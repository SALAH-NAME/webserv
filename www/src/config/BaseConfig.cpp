#include "BaseConfig.hpp"
#include "ConfigTypes.hpp"
#include "DefaultConfig.hpp"
#include <set>
#include <string>

BaseConfig::BaseConfig()
		: _client_max_body_size("0"), _root(""), _autoindex(false)
{
	/*_index.push_back(DefaultConfig::INDEX[0]);*/
	/*_allowed_methods.insert(HTTP_GET);*/
	/*_allowed_methods.insert(HTTP_POST);*/
	/*_allowed_methods.insert(HTTP_DELETE);*/
}

BaseConfig::~BaseConfig() {}

void BaseConfig::setErrorPage(int status_code, const std::string& path)
{
	_error_pages[status_code] = path;
}

void BaseConfig::setErrorPages(const std::map<int, std::string>& error_pages)
{
	_error_pages = error_pages;
}

void BaseConfig::setClientMaxBodySize(const std::string& size)
{
	_client_max_body_size = SizeValue(size);
}

void BaseConfig::setRoot(const std::string& root) { _root = root; }

void BaseConfig::setIndex(const std::vector<std::string>& index)
{
	_index = index;
}

void BaseConfig::addIndex(const std::string& index) { _index.push_back(index); }

void BaseConfig::setAllowedMethods(const std::set<HttpMethod>& methods)
{
	/*_allowed_methods.clear();*/
	_allowed_methods = methods;
}

void BaseConfig::addAllowedMethod(HttpMethod method)
{
	_allowed_methods.insert(method);
}

void BaseConfig::setAutoindex(bool autoindex) { _autoindex = autoindex; }

void BaseConfig::setUploadStore(const std::string& path)
{
	_upload_store = path;
}

const std::map<int, std::string>& BaseConfig::getErrorPages() const
{
	return _error_pages;
}

std::string BaseConfig::getErrorPage(int status_code) const
{
	std::map<int, std::string>::const_iterator it =
			_error_pages.find(status_code);
	if (it != _error_pages.end())
		return it->second;
	return "";
}

size_t BaseConfig::getClientMaxBodySize() const
{
	return _client_max_body_size.getBytes();
}

const std::string& BaseConfig::getRoot() const { return _root; }

const std::vector<std::string>& BaseConfig::getIndex() const { return _index; }

const std::set<HttpMethod>& BaseConfig::getAllowedMethods() const
{
	return _allowed_methods;
}

bool BaseConfig::isMethodAllowed(HttpMethod method) const
{
	return _allowed_methods.find(method) != _allowed_methods.end();
}

bool BaseConfig::getAutoindex() const { return _autoindex; }

const std::string& BaseConfig::getUploadStore() const { return _upload_store; }

void BaseConfig::inheritFrom(const BaseConfig& parent)
{
	if (_error_pages.empty())
		_error_pages = _error_pages = parent.getErrorPages();
	if (_client_max_body_size.getBytes() == 0)
		_client_max_body_size = SizeValue(to_string(parent.getClientMaxBodySize()));
	if (_root.empty())
		_root = parent.getRoot();
	if (_index.empty())
		_index = parent.getIndex();
	if (_allowed_methods.empty())
		_allowed_methods = parent.getAllowedMethods();
	if (_upload_store.empty())
		_upload_store = parent.getUploadStore();
}
