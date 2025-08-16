#include "ServerManager.hpp"

#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <cctype>
#include "Pipe.hpp"
#include "Environment.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"

#define SRV_NAME "Ed, Edd n Eddy/1.0"//or use webserv instead


#define HEADER_NAME_SIZE_LIMIT	256
#define HEADER_VALUE_SIZE_LIMIT	4000
#define HEADER_SIZE_LIMIT		32000
#define HEADERS_NUMBER_LIMIT    100


class ResponseHandler;

class CgiHandler
{
	private:
		bool								is_POST;
		Pipe								output_pipe;
		Pipe								input_pipe;
		Environment							env;
		char 								prev_buf_l_char;//last char from buffer
		int									child_pid;
		bool								Body_phase;
		std::time_t							exec_t0;
		std::map<std::string, std::string>	output_headers;
		int									parsed_bytes_count;
		std::vector<std::string>			extra_cookie_values;
		int									status_code;
		int									content_length;
		std::string							status_reason_phrase;
		std::string							preserved_body;
		std::string							key_holder;
		std::string							value_holder;

		bool	ReachedMaxHeadersNumber();
		void	HandleDuplicates();
		void	SetCgiChildFileDescriptors();
		void	SetCgiEnvironment(HttpRequest	&http_req, const ServerConfig &conf,
						ClientInfos &client_info);
		void	StatusValidator();
		void	AddNewHeader();
		void	ClearData();
		void	ContentLengthValidator();

	public:
		CgiHandler();
		void								RunCgi(HttpRequest &current_req, const ServerConfig &conf,
													const	LocationConfig &cgi_conf, std::string &script_path,
														ClientInfos &client_info);
		pid_t 								GetChildPid();
		void								ParseOutputBuffer(const std::string &new_buff);
		Pipe& 								GetInPipe();
		Pipe& 								GetOutPipe();
		std::string							GetPreservedBody();
		bool								ReachedBodyPhase();
		std::time_t							GetExecutionStartTime();
		std::map<std::string, std::string>&	GetOutputHeaders();
		int									GetStatusCode();
		std::string							GetReasonPhrase();
		int									GetContentLength();
		void								KillChild();
		bool								PostReq();
		void								PreBodyPhraseChecks();
		std::vector<std::string>			&GetExtraCookieValues();
		void								SyntaxErrorsCheck(const std::string &buff,
													unsigned int i, bool key_phase);
		~CgiHandler();
	
		class BadCgiOutput : public std::exception
		{
			private:
				std::string error;

			public:
				BadCgiOutput(const std::string &err_msg);
				const char *what() const throw();
				~BadCgiOutput() throw();
		};
};

std::string		NumtoString(int num);
bool			isAllDigit(std::string &str);
bool			Crlf_check(const std::string &str, unsigned int index);
void			TrimSpaces(std::string &str);
std::string		GetFileDirectoryPath(const std::string &path);
#endif
