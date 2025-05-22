#ifndef REQUEST_TMP_HPP
#define REQUEST_TMP_HPP

#include <string>
#include <vector>

class Request{
	public:
		std::string	method;
		std::string	script;
		std::string	interpiter;
		std::string path_info;
		std::string query_string;
		std::string content_type;
		std::string content_length;
		std::string server_name;
		int			server_port;
		std::string client_addrs;
		int 		client_port;
		std::vector<std::pair<std::string, std::string> > headers;//this vector should not include the content-type nor content-length
};

#endif