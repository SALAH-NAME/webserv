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
		ServerConfig			&conf;
		std::string				response_header;
		std::string 			resource_path;
		bool					require_cgi;
		CgiHandler				CgiObj;
		STRINGS_MAP				content_types;
		std::string				response_body;
		LocationConfig const	*loc_config;
		
		void 		RouteResolver(const std::string &path, const std::string &method);
		bool 		CheckForCgi(const std::string &req_path, LOCATIONS &srv_locations);
		void 		ProccessHttpGET(Request &req);
		void 		ProccessHttpPOST(Request &req);
		void 		ProccessHttpDELETE(Request &req);
		std::string GenerateContentType(const std::string file_extension);
		void 		GenerateDirListing(Request &req);
		bool 		NeedToRedirect(Request &req);
		void 		GenerateRedirection(Request &Req);
		void ResponseHandler::SetResponseHeader(Request &req, const std::string &status_line, int len,
						std::string location = "");

	public:
		ResponseHandler(int sockfd, ServerConfig &server_conf);
		void	ProccessRequest(Request &req);
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
bool 		IsDir(const char  *path);
std::string	ExtractFileExtension(const std::string &path);
std::string	GenerateTimeStamp();
std::string	NumtoString(int num);
std::string	formatDate(const char *format, time_t time, int len);

#endif