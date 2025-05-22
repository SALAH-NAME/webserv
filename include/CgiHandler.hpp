#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include "Request.hpp"
#include "Environment.hpp"

class CgiHandler{// a class made to abstract working with the cgi
	private:
		int			*IO_pipe;
		Request		&req;
		Environment	env;
		int			child_pid;
	public:
		CgiHandler(Request &http_req);
		void RunCgi();
		int GetChildPid();
		int *GetPipe();
		~CgiHandler();
};

#endif