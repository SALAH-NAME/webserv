#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <ctime>
#include "Request.hpp"
#include "Environment.hpp"
#include "ServerConfig.hpp"

class CgiHandler{// a class made to abstract working with the cgi
	private:
		bool		is_POST;
		int			*output_pipe;// used to pass data from cgi to server
		int			*input_pipe;// used to pass data from server to cgi (pass the body of a POST req)
		Environment	env;
		int			child_pid;
		std::time_t	exec_t0;// time passed since epoch to the time of execution

	public:
		CgiHandler();
		void RunCgi(Request &current_req, ServerConfig &conf,
					const LocationConfig &cgi_conf, std::string &script_path);
		pid_t GetChildPid();
		int *GetInPipe();
		int *GetOutPipe();
		~CgiHandler();
};

#endif