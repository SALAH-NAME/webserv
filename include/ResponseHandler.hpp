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
		ServerConfig				*conf;
		ClientInfos					client_info;
		HttpRequest					*req;
		std::string					response_header;
		std::string 				resource_path;
		int							child_status;
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
		int							cgi_tmpfile_id;
		bool						keep_alive;
		ServerManager 				*srv_mem_ptr;

		void		InitialRequestCheck();
		void		SetKeepAlive(); 
		void		ProccessRequest();
		void 		RouteResolver(const std::string &path, const std::string &method);
		bool 		CheckForCgi(const std::string &req_path, LOCATIONS &srv_locations);
		void 		InitializeStandardContentTypes();
		void		InitializeStatusPhrases();
		void	    HandleDirRequest();
		void 		ProccessHttpGET();
		void 		ProccessHttpPOST();
		void 		ProccessHttpDELETE();
		void		RefreshData();
		std::string GenerateContentType(const std::string file_extension);
		void		LoadStaticFile(const std::string &file_path, std::string status_line = "");
		void 		GenerateDirListing();
		bool 		NeedToRedirect();
		void 		GenerateRedirection();
		void		GenerateErrorPage( const std::string &status_line);
		std::string GenerateCgiStatusLine();
		void   		MakeLocationFromSrvConf();
		void		GenerateHeaderFromCgiData();
		void 		SetResponseHeader( const std::string &status_line, int len,
						bool is_static, std::string location = "");
		bool		CheckCgiTimeOut();
		void		UpdateCgiChildExitStatus();
			
	public:
		ResponseHandler(ServerManager *ptr);
		void			SetServerConf(ServerConfig *usedCon, const ClientInfos &clientInfos);
		void			CheckForContentType();			void			LoadErrorPage(const std::string &status_line, int status_code);
		void 			Run(HttpRequest &request);
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
		bool			KeepConnectioAlive();
		void			AppendCgiOutput(const std::string &buffer);
		void			FinishCgiResponse();
		void			CheckCgiChildState();
		void			SetTargetFileForCgi(int id);
		void			AppendBufferToTmpFile(const std::string &buf);
		void			DeleteCgiTargetFile();
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