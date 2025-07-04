
// ***********  temp file *****************************************
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>

class HttpRequest {
private:
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;

public:
    std::string getMethod() const;
    std::string getUri() const;
    std::string getVersion() const;
    std::map<std::string, std::string> getHeaders() const;

    bool parse(const std::string& raw_request);

    void printInfos() const;
};

#endif

// ***************************************************************
