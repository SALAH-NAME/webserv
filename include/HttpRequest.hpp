#pragma once
#include <string>
#include <map>
#include <sstream>
#include <algorithm>

// ***********  temp file *****************************************

class HttpRequest {
	private:
		std::string method;
		std::string uri;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
	public:
		std::string	getMethod(void);
		std::string	getUri(void);
		std::string	getVersion(void);
		std::map<std::string, std::string>	getHeaders(void);
		bool parse(const std::string& raw_request);
};

// ***************************************************************
