#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <iostream>

class Request
{
    private:
        bool        cgi_required;
        std::string	method;
		std::string	path;
		std::string http_version;
		std::string	interpiter;//usually in the configuration
		std::string path_info;
		std::string query_string;
		std::string content_type;
		std::string content_length;
		std::string server_name;//config
		int			server_port;//config
		std::string client_addrs;//config
		int 		client_port;//config
		std::map<std::string, std::string> headers;//exclude content-type and content-length
    public:
        void set_method(const std::string &method);
        void set_path(const std::string &path);
        void set_http_version(const std::string &http_version);
        void set_interpiter(const std::string &interpiter);
        void set_path_info(const std::string &path_info);
        void set_query_string(const std::string &query_string);
        void set_content_type(const std::string &content_type);
        void set_content_length(const std::string &content_length);
        void set_server_name(const std::string &server_name);
        void set_server_port(int server_port);
        void set_client_addrs(const std::string &client_addrs);
        void set_client_port(int client_port);
        void set_headers(const std::map<std::string, std::string> &headers);
        void set_cgi_required(bool cgi_required);


        std::string getMethod();
        std::string getPath();
        std::string getHttpVersion();
        std::string getInterpiter();
        std::string getPathInfo();
        std::string getQueryString();
        std::string getContentType();
        std::string getContentLength();
        std::string getServerName();
        int getServerPort();
        std::string getClientAddrs();
        int getClientPort();
        std::map<std::string, std::string> &getHeaders();
        bool require_cgi();
    };

#endif