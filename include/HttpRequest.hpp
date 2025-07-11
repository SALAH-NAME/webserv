
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

class HttpRequestException : public std::exception {
private:
    int _status_code;
    std::string _msg;
public:
    HttpRequestException(int code, const std::string& msg) : _status_code(code), _msg(msg) {}
    virtual ~HttpRequestException() throw() {}
    int statusCode() const { return _status_code; }
    const char* what() const throw() { return _msg.c_str(); }
};

class HttpRequest {
private:
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string path;
    std::string query_string;
    std::map<std::string, std::vector<std::string> > query_params;
    std::string path_info;
    bool valid;
    int status_code; // HTTP status code for parse errors

    void reset();
    void parseStartLine(const std::string& line);
    void parseHeaders(std::istream& stream);
    void parseBody(std::istream& stream);

public:
    HttpRequest();
    std::string getMethod() const;
    std::string getUri() const;
    std::string getVersion() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getBody() const;
    std::string getPath() const;
    std::string getQueryString() const;
    std::map<std::string, std::vector<std::string> > getQueryParams() const;
    std::string getPathInfo() const;
    bool isValid() const;
    int getStatusCode() const;

    void setMethod(const std::string& m);
    void setUri(const std::string& u);
    void setVersion(const std::string& v);
    void setHeaders(const std::map<std::string, std::string>& h);
    void setBody(const std::string& b);
    void setPath(const std::string& p);
    void setQueryString(const std::string& q);
    void setPathInfo(const std::string& pi);

    bool parse(const std::string& raw_request);

    void printInfos() const;
    std::map<std::string, std::string> getCookies() const;
    std::string getCookieValue(const std::string& key) const;
};

#endif
