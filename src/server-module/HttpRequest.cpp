#include "HttpRequest.hpp"


bool HttpRequest::parse(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string line;

    // 1. Parse the request line
    if (!std::getline(stream, line) || line.empty()) return false;
    std::istringstream req_line(line);
    if (!(req_line >> method >> uri >> version)) return false;

    // 2. Parse headers
    while (std::getline(stream, line) && line != "\r") {
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Trim whitespace
        key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        headers[key] = value;
    }

    // 3. Parse body (if Content-Length is present)
    std::string content_length_str = headers["Content-Length"];
    if (!content_length_str.empty()) {
        int content_length = std::stoi(content_length_str);
        body.resize(content_length);
        stream.read(&body[0], content_length);
    }

    return true;
}

std::string	HttpRequest::getMethod(void) {
    return method;
}

std::string	HttpRequest::getUri(void) {
    return uri;
}
std::string	HttpRequest::getVersion(void) {
    return version;
}

std::map<std::string, std::string>	HttpRequest::getHeaders(void) {
    return headers;
}