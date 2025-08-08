#ifndef BASE_CONFIG_HPP
#define BASE_CONFIG_HPP

#include "ConfigTypes.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

class BaseConfig
{
protected:
	std::map<int, std::string> _error_pages;
	SizeValue									 _client_max_body_size;
	std::string								 _root;
	std::vector<std::string>	 _index;
	std::set<HttpMethod>			 _allowed_methods;
	bool											 _autoindex;

	BaseConfig();

public:
	virtual ~BaseConfig();
	void setErrorPage(int status_code, const std::string& path);
	void setErrorPages(const std::map<int, std::string>& error_pages);
	void setClientMaxBodySize(const std::string& size);
	void setRoot(const std::string& root);
	void setIndex(const std::vector<std::string>& index);
	void addIndex(const std::string& index);
	void setAllowedMethods(const std::set<HttpMethod>& methods);
	void addAllowedMethod(HttpMethod method);
	void setAutoindex(bool autoindex);
	const std::map<int, std::string>& getErrorPages() const;
	std::string												getErrorPage(int status_code) const;
	size_t														getClientMaxBodySize() const;
	const std::string&								getRoot() const;
	const std::vector<std::string>&		getIndex() const;
	const std::set<HttpMethod>&				getAllowedMethods() const;
	bool															isMethodAllowed(HttpMethod method) const;
	bool															getAutoindex() const;
	virtual void											inheritFrom(const BaseConfig& parent);
};

#endif
