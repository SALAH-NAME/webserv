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
#include "Pipe.hpp"
#include "HttpRequest.hpp"
#include "CgiHandler.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"

typedef const std::map<std::string, LocationConfig> LOCATIONS;
typedef std::map<std::string, std::vector<std::string> > STRINGS_MAP;

#define CRLF "\r\n"
#define SRV_NAME "Ed, Edd n Eddy/1.0"//or use webserv instead
#define TMP_FILE_PREFIX "/tmp/tmp-cgi-body-holder_"

class ResponseHandler 
{
	private:
		const ServerConfig			&conf;
		std::string					remote_address;
		std::string					response_header;
		std::string 				resource_path;
		bool						require_cgi;
		bool						cgi_running;
		bool						is_post;
		bool						is_location_allocated;
		CgiHandler					CgiObj;
		STRINGS_MAP					content_types;
		std::map<int, std::string>	status_phrases;
		std::string					response_body;
		LocationConfig const		*loc_config;
		std::fstream				*target_file;
		unsigned int				cgi_buffer_size;

		void		InitialRequestCheck(HttpRequest &req);
		void		ProccessRequest(HttpRequest &req);
		void 		RouteResolver(const std::string &path, const std::string &method);
		bool 		CheckForCgi(const std::string &req_path, LOCATIONS &srv_locations);
		void 		InitializeStandardContentTypes();
		void		InitializeStatusPhrases();
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
		std::string GenerateCgiStatusLine();
		void   		MakeLocationFromSrvConf();
		void		GenerateHeaderFromCgiData();
		void 		SetResponseHeader(const std::string &status_line, int len,
						bool is_static, std::string location = "");
			
	public:
		ResponseHandler(const std::string &client_address, const ServerConfig &server_conf);
		void			LoadErrorPage(const std::string &status_line, int status_code);
		void 			Run(HttpRequest &req);
		bool			IsPost();		
		Pipe			&GetCgiInPipe();
		Pipe			&GetCgiOutPipe();
		std::string		GetResponseHeader();
		std::string		GetResourcePath();
		std::string 	GetResponseBody();
		std::fstream	*GetTargetFilePtr();
		pid_t			GetCgiChildPid();
		bool			RequireCgi();//only flags that cgiobj.run() is used
		bool			IsCgiChildRunning();
		bool			ReachedCgiBodyPhase();
		bool			CheckCgiTimeOut();
		void			AppendCgiOutput(const std::string &buffer);
		int				GetCgiChildExitStatus();
		void			FinishCgiResponse();
		void			CheckCgiChildState();
		void			SetTargetFileForCgi(int count);
		void			AppendBufferToTmpFile(const std::string &buf);

		~ResponseHandler();

		class ResponseHandlerError : public std::exception
		{
			private:
				int 		status_code;
				std::string error;
			public:
				int getStatusCode();
				ResponseHandlerError(const std::string &Errmsg, int statusCode);
				const char *what() const throw();
				~ResponseHandlerError() throw();
		};
};

//util functions
bool 		IsDir(const char  *path);
std::string	ExtractFileExtension(const std::string &path);
std::string	GenerateTimeStamp();
std::string	NumtoString(int num);
std::string	formatDate(const char *format, time_t time, int len);
std::string GetFileDirectoryPath(const std::string &path);

#endif