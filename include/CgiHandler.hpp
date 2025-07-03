#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <ctime>
#include "Request.hpp"
#include "Environment.hpp"

class CgiHandler{// a class made to abstract working with the cgi
	private:
		bool		is_POST;// true if we're working with http POST req
		int			*output_pipe;// used to pass data from cgi to server
		int			*input_pipe;// used to pass data from server to cgi (used to pass the body of a POST req)
 		Request		&req;
		Environment	env;// a class made to handle adding elements to the environment
		int			child_pid;// store the pid of the child in which the cgi-script will be executed
		std::time_t	exec_t0;// time passed since epoch to the time of execution
	public:
		CgiHandler(Request &http_req);
		void RunCgi();//spawns a child and set its environment based on the request data and execve the script it will use output_pipe[0] as its stdout
						//its input is accessible in the output_pipe[1] fd
		int GetChildPid();
		int *GetInPipe();
		int *GetOutPipe();
		~CgiHandler();
};

#endif