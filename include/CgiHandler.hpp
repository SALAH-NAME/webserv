#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <ctime>
#include "Pipe.hpp"
#include "Environment.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"

// Max header name length: 256 bytes
// Max header value length: 4KB 
// Max total headers size: 8KB
// Max number of headers: 100

class ResponseHandler;

class CgiHandler{
	private:
		bool		is_POST;
		Pipe		output_pipe;
		Pipe		input_pipe;
		Environment	env;
		int			child_pid;
		std::time_t	exec_t0;
		void	SetCgiChildFileDescriptors();
		void	SetCgiEnvironment(HttpRequest	&http_req, const ServerConfig &conf);

	public:
		CgiHandler();
		void RunCgi(HttpRequest &current_req, const ServerConfig &conf,
					const LocationConfig &cgi_conf, std::string &script_path);
		pid_t 	GetChildPid();
		Pipe 	&GetInPipe();
		Pipe 	&GetOutPipe();
		~CgiHandler();
};

std::string NumtoString(int num);

#endif