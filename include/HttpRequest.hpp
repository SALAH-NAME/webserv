
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <cstdlib>

class HttpRequestException : public std::exception
{
private:
    int _status_code;
    std::string _msg;

public:
    HttpRequestException(int code, const std::string &msg) : _status_code(code), _msg(msg) {}
    virtual ~HttpRequestException() throw() {}
    int statusCode() const { return _status_code; }
    const char *what() const throw() { return _msg.c_str(); }
};

class HttpRequest
{
public:
    enum State
    {
        STATE_START_LINE,
        STATE_HEADERS,
        STATE_BODY,
        STATE_COMPLETE,
        STATE_ERROR
    };

    void reset();

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

    State state;
    bool valid;
    int status_code;
    std::string error_msg;

    size_t _start_line_size;
    size_t _headers_size;
    bool _start_line_parsed;

    static const size_t MAX_START_LINE_SIZE = 2048;
    static const size_t MAX_HEADERS_SIZE = 8192;
    static const size_t MAX_HEADER_COUNT = 100;

    void parseStartLine(const std::string &line);
    void parseHeaderLine(const std::string &line);
    void validateStartLine(const std::string &line);
    void validateHeaderLine(const std::string &line);
    std::string toLower(const std::string &str) const;
    std::string trim(const std::string &str) const;
    void parseUriComponents();
    bool isValidMethod(const std::string &method) const;
    bool isValidVersion(const std::string &version) const;

public:
    HttpRequest();

    void appendAndValidate(std::string& _parsing_buffer);
    bool hasCompleteRequest() const;

    State getState() const;
    std::string getErrorMsg() const;
    bool isValid() const;
    int getStatusCode() const;

    std::string getMethod() const;
    std::string getUri() const;
    std::string getVersion() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getBody() const;
    std::string getPath() const;
    std::string getQueryString() const;
    std::map<std::string, std::vector<std::string> > getQueryParams() const;
    std::string getPathInfo() const;

    void setMethod(const std::string &m);
    void setUri(const std::string &u);
    void setVersion(const std::string &v);
    void setHeaders(const std::map<std::string, std::string> &h);
    void setBody(const std::string &b);
    void setPath(const std::string &p);
    void setQueryString(const std::string &q);
    void setPathInfo(const std::string &pi);

    void printInfos() const;
    std::map<std::string, std::string> getCookies() const;
    std::string getCookieValue(const std::string &key) const;
};

#endif
