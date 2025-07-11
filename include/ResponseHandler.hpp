#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <cstdio>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <sstream>
#include <dirent.h>
#include "HttpRequest.hpp"
#include "CgiHandler.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"

typedef const std::map<std::string, LocationConfig> LOCATIONS;
typedef std::map<std::string, std::vector<std::string> > STRINGS_MAP;

#define CRLF "\r\n"
#define SRV_NAME "Ed, Edd n Eddy/1.0"//or use webserv instead

class ResponseHandler 
{
	private:
		const ServerConfig		&conf;
		std::string				response_header;
		std::string 			resource_path;
		bool					require_cgi;
		bool					is_post;//true in case of POST but does not require cgi
		CgiHandler				CgiObj;
		STRINGS_MAP				content_types;
		std::string				response_body;
		LocationConfig const	*loc_config;
		std::fstream			*target_file;
	
		void		CheckForInitialErrors(HttpRequest &req);
		void		ProccessRequest(HttpRequest &req);
		void 		RouteResolver(const std::string &path, const std::string &method);
		bool 		CheckForCgi(const std::string &req_path, LOCATIONS &srv_locations);
		void 		InitializeStandardContentTypes();
		void 		ProccessHttpGET(HttpRequest &req);
		void 		ProccessHttpPOST(HttpRequest &req);
		void 		ProccessHttpDELETE(HttpRequest &req);
		std::string GenerateContentType(const std::string file_extension);
		void		LoadStaticFile(HttpRequest &req, const std::string &file_path);
		void 		GenerateDirListing(HttpRequest &req);
		bool 		NeedToRedirect(HttpRequest &req);
		void 		GenerateRedirection(HttpRequest &Req);
		void		GenerateErrorPage(const std::string &status_line, HttpRequest &req);
		void 		SetResponseHeader(HttpRequest &req, const std::string &status_line, int len,
			std::string location = "");
			
	public:
		ResponseHandler(const ServerConfig &server_conf);
		void			LoadErrorPage(const std::string &status_line, int status_code, HttpRequest &req);
		void 			Run(HttpRequest &req);
		bool			IsPost();		
		int				*GetCgiInPipe();
		int				*GetCgiOutPipe();
		std::string		GetResponseHeader();
		std::string 	GetResponseBody();
		std::fstream	*GetTargetFilePtr();
		pid_t			GetCgiChildPid();
		~ResponseHandler();

		class ResponseHandlerError : std::exception
		{
			private:
				int 		status_code;
				std::string error;
			public:
				int getStatusCode();
				ResponseHandlerError(const std::string &Errmsg, int statusCode);
				const char *what() throw();
				~ResponseHandlerError() throw();
		};
};

//util functions
bool 		IsDir(const char  *path);
std::string	ExtractFileExtension(const std::string &path);
std::string	GenerateTimeStamp();
std::string	NumtoString(int num);
std::string	formatDate(const char *format, time_t time, int len);

#endif