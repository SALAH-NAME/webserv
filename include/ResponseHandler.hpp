#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <cstdio>
#include <iomanip>
#include <sys/stat.h>
#include <algorithm>
#include <sstream>
#include <dirent.h>
#include "Request.hpp"
#include "CgiHandler.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"

typedef const std::map<std::string, LocationConfig> LOCATIONS;
typedef std::map<std::string, std::vector<std::string>> STRINGS_MAP;

#define CRLF "\r\n"
#define SRV_NAME "Ed, Edd n Eddy/1.0"//or use webserv instead

class ResponseHandler {
	private:
		int						socket_fd;
		std::string				response_header;
		std::string 			resource_path;
		bool					require_cgi;
		CgiHandler				CgiObj;
		STRINGS_MAP				content_types;
		std::string				response_body;
		LocationConfig const	*loc_config;
		
		void RouteResolver(const std::string &path, ServerConfig &conf, const std::string &method);
		bool CheckForCgi(const std::string &req_path, LOCATIONS &srv_locations);
		void ProccessHttpGET(Request &req, ServerConfig &conf);
		void ProccessHttpPOST(Request &req, ServerConfig &conf);
		void ProccessHttpDELETE(Request &req, ServerConfig &conf);
		void SetResponseHeader(Request &req, ServerConfig &conf, const std::string &);
		std::string GenerateContentType(const std::string file_extension);
		void GenerateDirListing(Request &req, ServerConfig &conf);
	public:
		ResponseHandler(int sockfd);
		void	ProccessRequest(Request &req, ServerConfig &conf);
		int		*GetCgiInPipe();
		int		*GetCgiOutPipe();
		pid_t	GetCgiChildPid();
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
bool 		is_dir(const char  *path);
std::string	ExtractFileExtension(const std::string &path);
std::string	GenerateTimeStamp();
std::string	NumtoString(int num);
std::string	formatDate(const char *format, time_t time, int len);

#endif