#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : valid(false), status_code(0) {}

std::string HttpRequest::getMethod() const { return method; }
std::string HttpRequest::getUri() const { return uri; }
std::string HttpRequest::getVersion() const { return version; }
std::map<std::string, std::string> HttpRequest::getHeaders() const { return headers; }
std::string HttpRequest::getBody() const { return body; }
std::string HttpRequest::getPath() const { return path; }
std::string HttpRequest::getQueryString() const { return query_string; }
std::map<std::string, std::vector<std::string> > HttpRequest::getQueryParams() const { return query_params; }
std::string HttpRequest::getPathInfo() const { return path_info; }
bool HttpRequest::isValid() const { return valid; }
int HttpRequest::getStatusCode() const { return status_code; }

void HttpRequest::setMethod(const std::string& m) { method = m; }
void HttpRequest::setUri(const std::string& u) { uri = u; }
void HttpRequest::setVersion(const std::string& v) { version = v; }
void HttpRequest::setHeaders(const std::map<std::string, std::string>& h) { headers = h; }
void HttpRequest::setBody(const std::string& b) { body = b; }
void HttpRequest::setPath(const std::string& p) { path = p; }
void HttpRequest::setQueryString(const std::string& q) { query_string = q; }
void HttpRequest::setPathInfo(const std::string& pi) { path_info = pi; }

static void parse_query_params(const std::string& query, std::map<std::string, std::vector<std::string> >& out) {
    out.clear();
    std::istringstream qstream(query);
    std::string pair;
    while (std::getline(qstream, pair, '&')) {
        std::string::size_type eq = pair.find('=');
        std::string key = (eq != std::string::npos) ? pair.substr(0, eq) : pair;
        std::string value = (eq != std::string::npos) ? pair.substr(eq + 1) : "";
        out[key].push_back(value);
    }
}

void HttpRequest::reset() {
    valid = false;
    status_code = 0;
    method.clear();
    uri.clear();
    version.clear();
    headers.clear();
    body.clear();
    path.clear();
    query_string.clear();
    path_info.clear();
    query_params.clear();
}

void HttpRequest::parseStartLine(const std::string& line) {
    std::istringstream start_line(line);
    if (!(start_line >> method >> uri >> version))
        throw HttpRequestException(400, "Malformed request line");
    if (version != "HTTP/1.1")
        throw HttpRequestException(505, "HTTP Version Not Supported");
    if (method != "GET" && method != "POST" && method != "DELETE")
        throw HttpRequestException(405, "Method Not Allowed");
    std::string::size_type qpos = uri.find('?');
    if (qpos != std::string::npos) {
        path = uri.substr(0, qpos);
        query_string = uri.substr(qpos + 1);
        parse_query_params(query_string, query_params);
    } else {
        path = uri;
        query_string.clear();
        query_params.clear();
    }
    std::string::size_type pinfo = path.find(';');
    if (pinfo != std::string::npos) {
        path_info = path.substr(pinfo + 1);
        path = path.substr(0, pinfo);
    }
}

void HttpRequest::parseHeaders(std::istream& stream) {
    std::string line;
    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty()) continue;
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

void HttpRequest::parseBody(std::istream& stream) {
    std::string line;
    std::string body_data;
    while (std::getline(stream, line)) {
        if (!body_data.empty()) body_data += "\n";
        body_data += line;
    }
    body = body_data;
}

bool HttpRequest::parse(const std::string& raw_request) {
    reset();
    std::istringstream stream(raw_request);
    std::string line;
    try {
        // State 1: Start line
        if (!std::getline(stream, line) || line.empty())
            throw HttpRequestException(400, "Empty request or missing start line");
        parseStartLine(line);
        // State 2: Headers
        parseHeaders(stream);
        // State 3: Body
        if (method == "POST") {
            bool hasContentLength = headers.find("Content-Length") != headers.end();
            bool hasChunked = false;
            std::map<std::string, std::string>::const_iterator it = headers.find("Transfer-Encoding");
            if (it != headers.end()) {
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
    } catch (const HttpRequestException& ex) {
        valid = false;
        status_code = ex.statusCode();
        return false;
    }
}

void HttpRequest::printInfos() const {
    std::cout << "Method: " << method << "\n";
    std::cout << "URI: " << uri << "\n";
    std::cout << "Version: " << version << "\n";
    std::cout << "Path: " << path << "\n";
    std::cout << "Query: " << query_string << "\n";
    std::cout << "PathInfo: " << path_info << "\n";
    std::cout << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Query Params: ";
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = query_params.begin(); it != query_params.end(); ++it) {
        std::cout << it->first << "=[";
        for (size_t i = 0; i < it->second.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << it->second[i];
        }
        std::cout << "] ";
    }
    std::cout << std::endl;
    std::cout << "Body: " << body << std::endl;
}

std::map<std::string, std::string> HttpRequest::getCookies() const {
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::string>::const_iterator it = headers.find("Cookie");
    if (it != headers.end()) {
        std::istringstream ss(it->second);
        std::string pair;
        while (std::getline(ss, pair, ';')) {
            std::string::size_type eq = pair.find('=');
            std::string key, value;
            if (eq != std::string::npos) {
                key = pair.substr(0, eq);
                value = pair.substr(eq + 1);
            } else {
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

std::string HttpRequest::getCookieValue(const std::string& key) const {
    std::map<std::string, std::string> cookies = getCookies();
    std::map<std::string, std::string>::const_iterator it = cookies.find(key);
    if (it != cookies.end())
        return it->second;
    return "";
}
