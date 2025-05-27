#include "Response.hpp"

int check_file(const std::string& path) {
    struct stat sb;

    if (stat(path.c_str(), &sb) != 0) {
        // File does not exist
        return 404;
    }

    // Check if it's a regular file (not directory or special file)
    if (!S_ISREG(sb.st_mode)) {
        if (path[path.size() - 1] == '/')
            return 403;
        return 301;
    }

    // Check read permission
    if (access(path.c_str(), R_OK) != 0) {
        return 403;
    }

    return 200;
}

bool containsUppercase(const std::string& str) {
    for (int i = 0; i < str.size(); i++) {
        if (!std::isupper(str[i])) {
            return false;
        }
    }
    return true;
}

bool startsWithHTTP(const std::string& str) {
    return str.size() >= 5 && str.compare(0, 5, "HTTP/") == 0;
}

void    setDate(std::string& Date) {
    std::time_t now = std::time(0);
	std::tm* gmt = std::gmtime(&now); // get time in GMT
	
	char buf[1024];
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);

    Date += buf;
}

