#ifndef CONFIG_TYPES_HPP
#define CONFIG_TYPES_HPP

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>

enum HttpMethod
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE
};

inline HttpMethod stringToHttpMethod(const std::string &method)
{
	if (method == "GET")
		return HTTP_GET;
	if (method == "POST")
		return HTTP_POST;
	if (method == "DELETE")
		return HTTP_DELETE;
	throw std::invalid_argument("Invalid HTTP method: " + method);
}

inline std::string HttpMethodToString(HttpMethod method)
{
	switch (method)
	{
	case HTTP_GET:
		return "GET";
	case HTTP_POST:
		return "POST";
	case HTTP_DELETE:
		return "DELETE";
	default:
		return "UNKNOWN";
	}
}

struct RedirectInfo
{
	int					status_code;
	std::string url;

	RedirectInfo();
	RedirectInfo(int code, const std::string &target);
	bool isValid() const;
};

class SizeValue
{
private:
	size_t _bytes;

public:
	SizeValue();
	SizeValue(const std::string &sizeStr);

	size_t getBytes() const;
	void	 parseSize(const std::string &sizeStr);

	bool operator<(const SizeValue &other) const;
	bool operator>(const SizeValue &other) const;
	bool operator==(const SizeValue &other) const;
};

template <typename T> std::string to_string(T value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

#endif
