
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include "Request.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"

class ResponseHandler {
	private:
		int			socket_fd;
		std::string resource_path;
		LocationConfig const *loc_config;

	public:
		ResponseHandler(int sockfd);
		void PreProccessRequest(Request &req, ServerConfig &conf);
		void RouteResolver(const std::string &path, ServerConfig &conf, const std::string &method);
		void ProccessHttpGET(Request &req, ServerConfig &conf);
		void ProccessHttpPOST(Request &req, ServerConfig &conf);
		void ProccessHttpDELETE(Request &req, ServerConfig &conf);
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

#endif