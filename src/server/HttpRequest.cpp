#include "HttpRequest.hpp"
#include <climits>

HttpRequest::HttpRequest() : state(STATE_START_LINE), valid(false), status_code(0),
                             http_version(HTTP_UNKNOWN), _start_line_size(0), _headers_size(0),
                             _start_line_parsed(false) {}

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
        result[i] = std::tolower(result[i]);
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

std::string HttpRequest::normalizeHostHeader(const std::string &host) const
{
    std::string::size_type colon_pos = host.find(':');
    if (colon_pos != std::string::npos)
        return host.substr(0, colon_pos);
    
    return host;
}

bool HttpRequest::isValidMethod(const std::string &method) const
{
    return method == "GET" || method == "POST" || method == "DELETE";
}

bool HttpRequest::isValidVersion(const std::string &version) const
{
    return version == "HTTP/1.1" || version == "HTTP/1.0";
}

HttpRequest::HttpVersion HttpRequest::parseHttpVersion(const std::string &version) const
{
    if (version == "HTTP/1.1")
        return HTTP_1_1;
    else if (version == "HTTP/1.0")
        return HTTP_1_0;
    else
        return HTTP_UNKNOWN;
}

HttpRequest::HttpVersion HttpRequest::getHttpVersion() const
{
    return http_version;
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
    http_version = parseHttpVersion(version_part);

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
    if (isDuplicateHeader(header_name))
        throw HttpRequestException(400, "Bad Request - Duplicate header");
    
    if (header_name == "host")
        header_value = normalizeHostHeader(header_value);
    
    headers[header_name] = header_value;
}

bool HttpRequest::isRequiredHeader(const std::string &headerName) const
{
    return (headerName == "host" || headerName == "content-length" || headerName == "transfer-encoding" || headerName == "connection");
}

bool HttpRequest::isDuplicateHeader(const std::string &headerName) const
{
    if (headers.find(headerName) != headers.end() && isRequiredHeader(headerName))
        return true;
    return false;
}

bool HttpRequest::isValidContentLength(const std::string &value) const
{
    if (value.empty())
        return false;

    std::string trimmed_value = trim(value);
    if (trimmed_value.empty())
        return false;

    for (size_t i = 0; i < trimmed_value.length(); ++i)
    {
        if (!std::isdigit(trimmed_value[i]))
            return false;
    }
    
    std::istringstream iss(trimmed_value);
    long long content_length;
    iss >> content_length;
    
    if (iss.fail() || !iss.eof() || content_length < 0)
        return false;
        
    return true;
}

bool HttpRequest::hasConflictingHeaders() const
{
    bool hasContentLength = headers.find("content-length") != headers.end();
    bool hasTransferEncoding = headers.find("transfer-encoding") != headers.end();

    if (hasContentLength && hasTransferEncoding)
    {
        std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
        if (it != headers.end() && isChunkedEncoding(it->second))
            return true;
    }
    return false;
}

void HttpRequest::validateVersionSpecificHeaders()
{
    if (http_version == HTTP_1_0)
        validateHttp10Requirements();
    else if (http_version == HTTP_1_1)
        validateHttp11Requirements();
}

void HttpRequest::validateHttp10Requirements()
{
    if (method == "POST")
    {
        bool hasContentLength = headers.find("content-length") != headers.end();
        bool hasTransferEncoding = headers.find("transfer-encoding") != headers.end();

        if (hasTransferEncoding)
            throw HttpRequestException(400, "HTTP/1.0 does not support Transfer-Encoding header");

        if (!hasContentLength)
            throw HttpRequestException(400, "HTTP/1.0 POST requests require Content-Length header");

        std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
        if (!isValidContentLength(it->second))
            throw HttpRequestException(400, "Invalid Content-Length value: must be a non-negative integer");
    }
}

void HttpRequest::validateHttp11Requirements()
{
    if (headers.find("host") == headers.end())
        throw HttpRequestException(400, "HTTP/1.1 requests require Host header");

    if (hasConflictingHeaders())
        throw HttpRequestException(400, "Message cannot contain both Content-Length and Transfer-Encoding: chunked");

    if (method == "POST")
    {
        bool hasContentLength = headers.find("content-length") != headers.end();
        bool hasChunked = false;

        std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
        if (it != headers.end())
            hasChunked = isChunkedEncoding(it->second);

        if (!hasContentLength && !hasChunked)
            throw HttpRequestException(411, "HTTP/1.1 POST requests require Content-Length or Transfer-Encoding: chunked");

        if (hasContentLength)
        {
            std::map<std::string, std::string>::const_iterator cl_it = headers.find("content-length");
            if (!isValidContentLength(cl_it->second))
                throw HttpRequestException(400, "Invalid Content-Length value: must be a non-negative integer");
        }
    }
}

void HttpRequest::validateContentLengthLimit(size_t max_body_size) const
{
    std::map<std::string, std::string>::const_iterator cl_it = headers.find("content-length");
    if (cl_it == headers.end())
        return;
        
    if (!isValidContentLength(cl_it->second))
        return;
        
    std::string trimmed_value = trim(cl_it->second);
    std::istringstream iss(trimmed_value);
    long long content_length;
    iss >> content_length;
    
    if (static_cast<size_t>(content_length) > max_body_size)
        throw HttpRequestException(413, "Request entity too large");
}

bool HttpRequest::shouldContinueParsing() const
{
    return state != STATE_ERROR && state != STATE_COMPLETE;
}

std::string HttpRequest::extractNextLine(std::string &buffer, std::string::size_type &pos, bool &found)
{
    std::string::size_type line_end = buffer.find("\r\n", pos);
    if (line_end == std::string::npos)
    {
        found = false;
        return "";
    }

    found = true;
    std::string line = buffer.substr(pos, line_end - pos);
    pos = line_end + 2; // Skip \r\n
    return line;        // Note: line can be empty (empty line) but found will be true
}

void HttpRequest::updateBufferAfterProcessing(std::string &buffer, std::string::size_type pos)
{
    if (pos > 0)
        buffer = buffer.substr(pos);
}

void HttpRequest::handleParsingError(const HttpRequestException &e)
{
    state = STATE_ERROR;
    valid = false;
    status_code = e.statusCode();
    error_msg = e.what();
}

void HttpRequest::processStartLine(const std::string &line)
{
    if (!_start_line_parsed)
    {
        validateStartLine(line);
        parseStartLine(line);
        _start_line_parsed = true;
        state = STATE_HEADERS;
    }
}

void HttpRequest::processHeaderLine(const std::string &line)
{
    if (line.empty())
        processEndOfHeaders();
    else
    {
        validateHeaderLine(line);
        parseHeaderLine(line);
    }
}

void HttpRequest::processEndOfHeaders()
{
    // Empty line so end of headers
    state = STATE_BODY; //  STATE_COMPLETE
    valid = true;
    status_code = 200;

    validateVersionSpecificHeaders();
}

void HttpRequest::validateRequiredHeaders()
{
    if (headers.find("host") == headers.end())
        throw HttpRequestException(400, "Missing Host header");
}

void HttpRequest::validatePostRequest()
{
    if (method == "POST")
    {
        bool hasContentLength = headers.find("content-length") != headers.end();
        bool hasChunked = false;
        std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
        if (it != headers.end())
        {
            hasChunked = isChunkedEncoding(it->second);
        }
        if (!hasContentLength && !hasChunked)
            throw HttpRequestException(411, "Length Required");
    }
}

bool HttpRequest::isChunkedEncoding(const std::string &transferEncoding) const
{
    std::string lowerTE = toLower(transferEncoding);
    std::string::size_type pos = lowerTE.find("chunked");
    return pos != std::string::npos;
}

bool HttpRequest::hasCompleteLineInBuffer(const std::string &buffer, std::string::size_type pos) const
{
    return buffer.find("\r\n", pos) != std::string::npos;
}

void HttpRequest::appendAndValidate(std::string &_parsing_buffer)
{
    if (!shouldContinueParsing())
        return;

    std::string::size_type pos = 0;
    std::string::size_type original_pos = 0;

    // Process all complete lines in the buffer
    while (hasCompleteLineInBuffer(_parsing_buffer, pos))
    {
        original_pos = pos;
        bool line_found = false;
        std::string line = extractNextLine(_parsing_buffer, pos, line_found);
        if (!line_found) // No complete line found
        {
            pos = original_pos; // Reset pos to where we started
            break;
        }

        try
        {
            if (state == STATE_START_LINE)
                processStartLine(line);
            else if (state == STATE_HEADERS)
            {
                processHeaderLine(line);
                if (state == STATE_BODY) // End of headers reached
                    break;
            }
        }
        catch (const HttpRequestException &e)
        {
            handleParsingError(e);
            throw;
        }
    }

    // Remove processed data from buffer
    updateBufferAfterProcessing(_parsing_buffer, pos);

    // state == STATE_BODY when headers are complete
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
    http_version = HTTP_UNKNOWN;

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
    std::map<std::string, std::string>::const_iterator it = headers.find("cookie");
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


bool HttpRequest::isCunked() const
{
    std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
    if (it != headers.end())
        return isChunkedEncoding(it->second);
    return false;
}

bool HttpRequest::isValidHexDigit(char c) const
{
    return (c >= '0' && c <= '9') || 
           (c >= 'A' && c <= 'F') || 
           (c >= 'a' && c <= 'f');
}

int HttpRequest::hexCharToDecimal(char c) const
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

std::size_t HttpRequest::findCRLF(const std::string& buffer) const
{
    if (buffer.empty())
        throw HttpRequestException(400, "Bad Request: Empty buffer");

    std::size_t crlf_pos = buffer.find("\r\n");
    if (crlf_pos == std::string::npos)
        throw HttpRequestException(400, "Bad Request: Missing CRLF in chunk size line");

    if (crlf_pos == 0)
        throw HttpRequestException(400, "Bad Request: Empty chunk size line");

    return crlf_pos;
}

std::string HttpRequest::extractHexString(const std::string& chunk_line) const
{
    std::size_t hex_end = 0;
    while (hex_end < chunk_line.length() && isValidHexDigit(chunk_line[hex_end]))
        hex_end++;

    if (hex_end == 0)
        throw HttpRequestException(400, "Bad Request: Invalid chunk size - no hex digits found");

    if (hex_end < chunk_line.length())
        throw HttpRequestException(400, "Bad Request: Invalid characters after chunk size");

    std::string hex_string = chunk_line.substr(0, hex_end);

    if (hex_string.length() > 8)
        throw HttpRequestException(413, "Payload Too Large: Chunk size too large");

    return hex_string;
}

long HttpRequest::convertHexToDecimal(const std::string& hex_string)
{
    long chunk_size = 0;
    
    for (std::size_t i = 0; i < hex_string.length(); i++)
    {
        int hex_val = hexCharToDecimal(hex_string[i]);
        if (hex_val == -1)
            throw HttpRequestException(400, "Bad Request: Invalid hex digit in chunk size");
        
        if (chunk_size > (LONG_MAX - hex_val) / 16)
            throw HttpRequestException(413, "Payload Too Large: Chunk size overflow");
        
        chunk_size = chunk_size * 16 + hex_val;
    }
    
    return chunk_size;
}

int HttpRequest::validateIntRange(long chunk_size) const
{
    if (chunk_size > INT_MAX)
        throw HttpRequestException(413, "Payload Too Large: Chunk size exceeds integer range");
    return static_cast<int>(chunk_size);
}

void HttpRequest::consumeChunkSizeLine(std::string& buffer, std::size_t crlf_pos) const
{
    buffer.erase(0, crlf_pos + 2); // +2 for \r\n
}

int HttpRequest::validateChunkSize(std::string& buffer)
{
    std::size_t crlf_pos = findCRLF(buffer);
    
    std::string chunk_line = buffer.substr(0, crlf_pos);
    
    std::string hex_string = extractHexString(chunk_line);
    
    long chunk_size_long = convertHexToDecimal(hex_string);
    
    int chunk_size = validateIntRange(chunk_size_long);
    
    consumeChunkSizeLine(buffer, crlf_pos);
    
    return chunk_size;
}