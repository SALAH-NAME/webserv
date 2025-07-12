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
		void	    HandleDirRequest(HttpRequest &req);
		void 		ProccessHttpGET(HttpRequest &req);
		void 		ProccessHttpPOST(HttpRequest &req);
		void 		ProccessHttpDELETE();
		std::string GenerateContentType(const std::string file_extension);
		void		LoadStaticFile(const std::string &file_path,
						const std::string &status_line = "HTTP/1.1 200 OK");
		void 		GenerateDirListing(HttpRequest &req);
		bool 		NeedToRedirect(HttpRequest &req);
		void 		GenerateRedirection(HttpRequest &req);
		void		GenerateErrorPage(const std::string &status_line);
		void 		SetResponseHeader(const std::string &status_line, int len,
						bool is_static, std::string location = "");
			
	public:
		ResponseHandler(const ServerConfig &server_conf);
		void			LoadErrorPage(const std::string &status_line, int status_code);
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