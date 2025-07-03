#include "HttpRequest.hpp"

std::string HttpRequest::getMethod() const {
    return method;
}

std::string HttpRequest::getUri() const {
    return uri;
}

std::string HttpRequest::getVersion() const {
    return version;
}

std::map<std::string, std::string> HttpRequest::getHeaders() const {
    return headers;
}

bool HttpRequest::parse(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string line;

    // Parse start line
    if (!std::getline(stream, line) || line.empty())
        return false;

    std::istringstream start_line(line);
    if (!(start_line >> method >> uri >> version))
        return false;

    // Parse headers
    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        std::string::size_type colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);

            // Trim whitespace
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
    }

    return true;
}

void HttpRequest::printInfos() const {
    std::cout << "Method: " << method << std::endl;
    std::cout << "URI: " << uri << std::endl;
    std::cout << "Version: " << version << std::endl;
    std::cout << "Headers:" << std::endl;

    std::map<std::string, std::string>::const_iterator it;
    for (it = headers.begin(); it != headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
}
