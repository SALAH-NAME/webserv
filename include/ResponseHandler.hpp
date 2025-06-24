
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include "Request.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"

typedef const std::map<std::string, LocationConfig> LOCATIONS;

class ResponseHandler {
	private:
		int						socket_fd;
		std::string 			resource_path;
		bool					require_cgi;
		LocationConfig const	*loc_config;
		
		void RouteResolver(const std::string &path, ServerConfig &conf, const std::string &method);
		bool CheckForCgi(const std::string &req_path, LOCATIONS &srv_locations);
		void ProccessHttpGET(Request &req, ServerConfig &conf);
		void ProccessHttpPOST(Request &req, ServerConfig &conf);
		void ProccessHttpDELETE(Request &req, ServerConfig &conf);

	public:
		ResponseHandler(int sockfd);
		void	ProccessRequest(Request &req, ServerConfig &conf);
		~ResponseHandler();

		class RequestError : std::exception
		{
			private:
				std::string error;
			public:
				RequestError(const std::string &Errmsg);
				const char *what() noexcept;
		};
};


//util functions
bool is_dir(const char  *path);
std::string ExtractFileExtension(const std::string &path);
#endif