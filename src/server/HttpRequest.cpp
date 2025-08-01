#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : state(STATE_START_LINE), valid(false), status_code(0),
                             _start_line_size(0), _headers_size(0), _start_line_parsed(false) {}

HttpRequest::State HttpRequest::getState() const
{
    return state;
}

std::string HttpRequest::getErrorMsg() const
{
    return error_msg;
}

bool HttpRequest::isValid() const
{
    return valid;
}

int HttpRequest::getStatusCode() const
{
    return status_code;
}

std::string HttpRequest::getMethod() const { return method; }
std::string HttpRequest::getUri() const { return uri; }
std::string HttpRequest::getVersion() const { return version; }
std::map<std::string, std::string> HttpRequest::getHeaders() const { return headers; }
std::string HttpRequest::getBody() const { return body; }
std::string HttpRequest::getPath() const { return path; }
std::string HttpRequest::getQueryString() const { return query_string; }
std::map<std::string, std::vector<std::string> > HttpRequest::getQueryParams() const { return query_params; }
std::string HttpRequest::getPathInfo() const { return path_info; }

void HttpRequest::setMethod(const std::string &m) { method = m; }
void HttpRequest::setUri(const std::string &u) { uri = u; }
void HttpRequest::setVersion(const std::string &v) { version = v; }
void HttpRequest::setHeaders(const std::map<std::string, std::string> &h) { headers = h; }
void HttpRequest::setBody(const std::string &b) { body = b; }
void HttpRequest::setPath(const std::string &p) { path = p; }
void HttpRequest::setQueryString(const std::string &q) { query_string = q; }
void HttpRequest::setPathInfo(const std::string &pi) { path_info = pi; }

std::string HttpRequest::toLower(const std::string &str) const
{
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i)
    {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

std::string HttpRequest::trim(const std::string &str) const
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool HttpRequest::isValidMethod(const std::string &method) const
{
    return method == "GET" || method == "POST" || method == "DELETE";
}

bool HttpRequest::isValidVersion(const std::string &version) const
{
    return version == "HTTP/1.1";
}

void HttpRequest::validateHeaderLine(const std::string &line)
{
    _headers_size += line.length() + 2; // +2 for \r\n

    if (_headers_size > MAX_HEADERS_SIZE)
        throw HttpRequestException(431, "Request Header Fields Too Large");

    if (headers.size() >= MAX_HEADER_COUNT)
        throw HttpRequestException(431, "Request Header Fields Too Large");

    std::string::size_type colon_pos = line.find(':');
    if (colon_pos == std::string::npos)
        throw HttpRequestException(400, "Bad Request - Malformed header line");

    std::string header_name = line.substr(0, colon_pos);
    std::string header_value = line.substr(colon_pos + 1);

    if (header_name.empty())
        throw HttpRequestException(400, "Bad Request - Empty header name");

    for (size_t i = 0; i < header_name.length(); ++i)
    {
        char c = header_name[i];
        if (!std::isalnum(c) && c != '-' && c != '_')
            throw HttpRequestException(400, "Bad Request - Invalid header name");
    }

    header_value = trim(header_value);

    for (size_t i = 0; i < header_value.length(); ++i)
    {
        char c = header_value[i];
        if (std::iscntrl(c) && c != '\t')
            throw HttpRequestException(400, "Bad Request - Invalid header value");
    }
}

void HttpRequest::validateStartLine(const std::string &line)
{
    _start_line_size += line.length() + 2; // +2 for \r\n

    if (_start_line_size > MAX_START_LINE_SIZE)
        throw HttpRequestException(414, "Request-URI Too Long");

    if (line.empty())
        throw HttpRequestException(400, "Bad Request - Empty start line");

    std::istringstream iss(line);
    std::string method_part, uri_part, version_part, extra;

    if (!(iss >> method_part >> uri_part >> version_part))
        throw HttpRequestException(400, "Bad Request - Malformed request line");

    if (iss >> extra)
        throw HttpRequestException(400, "Bad Request - Invalid request line format");

    if (!isValidMethod(method_part))
        throw HttpRequestException(405, "Method Not Allowed");

    if (!isValidVersion(version_part))
        throw HttpRequestException(505, "HTTP Version Not Supported");

    if (uri_part.empty() || uri_part[0] != '/')
        throw HttpRequestException(400, "Bad Request - Invalid URI");

    if (uri_part.length() > 2048)
        throw HttpRequestException(414, "Request-URI Too Long");
}

void HttpRequest::parseStartLine(const std::string &line)
{
    // Remove trailing \r if present
    std::string cleaned_line = line;
    if (!cleaned_line.empty() && cleaned_line[cleaned_line.length() - 1] == '\r')
        cleaned_line.erase(cleaned_line.length() - 1);

    if (cleaned_line.empty())
        throw HttpRequestException(400, "Empty request line");

    std::istringstream iss(cleaned_line);
    std::string method_part, uri_part, version_part, extra;

    if (!(iss >> method_part >> uri_part >> version_part))
        throw HttpRequestException(400, "Malformed request line");

    if (iss >> extra)
        throw HttpRequestException(400, "Malformed request line");

    if (!isValidMethod(method_part))
        throw HttpRequestException(405, "Method Not Allowed");

    if (!isValidVersion(version_part))
        throw HttpRequestException(505, "HTTP Version Not Supported");

    if (uri_part.empty() || uri_part[0] != '/')
        throw HttpRequestException(400, "Bad Request - Invalid URI");

    method = method_part;
    uri = uri_part;
    version = version_part;

    parseUriComponents();
}

// Parse URI into path, query string, and path info
void HttpRequest::parseUriComponents()
{
    std::string::size_type query_pos = uri.find('?');
    if (query_pos != std::string::npos)
    {
        path = uri.substr(0, query_pos);
        query_string = uri.substr(query_pos + 1);

        query_params.clear();
        std::istringstream query_stream(query_string);
        std::string pair;
        while (std::getline(query_stream, pair, '&'))
        {
            std::string::size_type eq_pos = pair.find('=');
            std::string key = (eq_pos != std::string::npos) ? pair.substr(0, eq_pos) : pair;
            std::string value = (eq_pos != std::string::npos) ? pair.substr(eq_pos + 1) : "";
            query_params[key].push_back(value);
        }
    }
    else
    {
        path = uri;
        query_string.clear();
        query_params.clear();
    }

    // std::string::size_type semicolon_pos = path.find(';');
    // if (semicolon_pos != std::string::npos)
    // {
    //     path_info = path.substr(semicolon_pos + 1);
    //     path = path.substr(0, semicolon_pos);
    // }
    // else
    //     path_info.clear();
}

void HttpRequest::parseHeaderLine(const std::string &line)
{
    std::string::size_type colon_pos = line.find(':');
    std::string header_name = trim(line.substr(0, colon_pos));
    std::string header_value = trim(line.substr(colon_pos + 1));

    // For case-insensitive comparison
    header_name = toLower(header_name);

    headers[header_name] = header_value;
}

void HttpRequest::appendAndValidate(std::string& _parsing_buffer)
{
    if (state == STATE_ERROR || state == STATE_COMPLETE)
        return;

    std::string::size_type pos = 0;
    std::string::size_type line_end;

    // Process complete lines (ending with \r\n)
    while ((line_end = _parsing_buffer.find("\r\n", pos)) != std::string::npos)
    {
        std::string line = _parsing_buffer.substr(pos, line_end - pos);

        try
        {
            if (state == STATE_START_LINE)
            {
                if (!_start_line_parsed)
                {
                    validateStartLine(line);
                    parseStartLine(line);
                    _start_line_parsed = true;
                    state = STATE_HEADERS;
                }
            }
            else if (state == STATE_HEADERS)
            {
                if (line.empty())
                {
                    // Empty line so end of headers
                    state = STATE_BODY; //  STATE_COMPLETE
                    valid = true;
                    status_code = 200;
                    break;
                }
                else
                {
                    validateHeaderLine(line);
                    parseHeaderLine(line);
                }
            }
        }
        catch (const HttpRequestException &e)
        {
            state = STATE_ERROR;
            valid = false;
            status_code = e.statusCode();
            error_msg = e.what();
            throw;
        }

        pos = line_end + 2; // Skip \r\n
    }

    // Remove processed lines from buffer
    if (pos > 0)
    {
        _parsing_buffer = _parsing_buffer.substr(pos);
    }
}

bool HttpRequest::hasCompleteRequest() const
{
    return state == STATE_COMPLETE;
}

void HttpRequest::reset()
{
    state = STATE_START_LINE;
    valid = false;
    status_code = 0;
    error_msg.clear();

    method.clear();
    uri.clear();
    version.clear();
    headers.clear();
    body.clear();
    path.clear();
    query_string.clear();
    path_info.clear();
    query_params.clear();

    // _parsing_buffer.clear();
    _start_line_size = 0;
    _headers_size = 0;
    _start_line_parsed = false;
}

void HttpRequest::parseHeaders(std::istream &stream)
{
    std::string line;
    while (std::getline(stream, line) && line != "\r")
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            continue;
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos)
            throw HttpRequestException(400, "Malformed header line");
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        std::string::size_type start = key.find_first_not_of(" \t");
        std::string::size_type end = key.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos)
            key = key.substr(start, end - start + 1);
        start = value.find_first_not_of(" \t");
        end = value.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos)
            value = value.substr(start, end - start + 1);
        headers[key] = value;
    }
    if (headers.find("Host") == headers.end())
        throw HttpRequestException(400, "Missing Host header");
}

void HttpRequest::parseBody(std::istream &stream)
{
    std::string line;
    std::string body_data;
    while (std::getline(stream, line))
    {
        if (!body_data.empty())
            body_data += "\n";
        body_data += line;
    }
    body = body_data;
}

bool HttpRequest::parse(const std::string &raw_request)
{
    reset();
    std::istringstream stream(raw_request);
    std::string line;
    try
    {
        // State 1: Start line
        if (!std::getline(stream, line))
            throw HttpRequestException(400, "Empty request or missing start line");

        // Remove \r if present and check if line is empty
        std::string cleaned_line = line;
        if (!cleaned_line.empty() && cleaned_line[cleaned_line.length() - 1] == '\r')
        {
            cleaned_line.erase(cleaned_line.length() - 1);
        }
        if (cleaned_line.empty())
            throw HttpRequestException(400, "Empty request or missing start line");

        parseStartLine(line);
        // State 2: Headers
        parseHeaders(stream);
        // State 3: Body
        if (method == "POST")
        {
            bool hasContentLength = headers.find("Content-Length") != headers.end();
            bool hasChunked = false;
            std::map<std::string, std::string>::const_iterator it = headers.find("Transfer-Encoding");
            if (it != headers.end())
            {
                std::string val = it->second;
                std::string::size_type pos = val.find("chunked");
                if (pos != std::string::npos)
                    hasChunked = true;
            }
            if (!hasContentLength && !hasChunked)
                throw HttpRequestException(411, "Length Required");
        }
        parseBody(stream);
        valid = true;
        status_code = 200;
        return true;
    }
    catch (const HttpRequestException &ex)
    {
        valid = false;
        status_code = ex.statusCode();
        return false;
    }
}

void HttpRequest::printInfos() const
{
    std::cout << "Method: " << method << "\n";
    std::cout << "URI: " << uri << "\n";
    std::cout << "Version: " << version << "\n";
    std::cout << "Path: " << path << "\n";
    std::cout << "Query: " << query_string << "\n";
    std::cout << "PathInfo: " << path_info << "\n";
    std::cout << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Query Params: ";
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = query_params.begin(); it != query_params.end(); ++it)
    {
        std::cout << it->first << "=[";
        for (size_t i = 0; i < it->second.size(); ++i)
        {
            if (i > 0)
                std::cout << ",";
            std::cout << it->second[i];
        }
        std::cout << "] ";
    }
    std::cout << std::endl;
    std::cout << "Body: " << body << std::endl;
}

std::map<std::string, std::string> HttpRequest::getCookies() const
{
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::string>::const_iterator it = headers.find("Cookie");
    if (it != headers.end())
    {
        std::istringstream ss(it->second);
        std::string pair;
        while (std::getline(ss, pair, ';'))
        {
            std::string::size_type eq = pair.find('=');
            std::string key, value;
            if (eq != std::string::npos)
            {
                key = pair.substr(0, eq);
                value = pair.substr(eq + 1);
            }
            else
            {
                key = pair;
                value = "";
            }
            std::string::size_type start = key.find_first_not_of(" \t");
            std::string::size_type end = key.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos)
                key = key.substr(start, end - start + 1);
            start = value.find_first_not_of(" \t");
            end = value.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos)
                value = value.substr(start, end - start + 1);
            cookies[key] = value;
        }
    }
    return cookies;
}

std::string HttpRequest::getCookieValue(const std::string &key) const
{
    std::map<std::string, std::string> cookies = getCookies();
    std::map<std::string, std::string>::const_iterator it = cookies.find(key);
    if (it != cookies.end())
        return it->second;
    return "";
}
