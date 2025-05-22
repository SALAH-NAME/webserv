#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include "Request.tmp.hpp"
#include "Environment.hpp"

class CgiHandler{// a class made to abstract working with the cgi
	private:
		Request req;
	public:
		CgiHandler(Request http_req, Environment my_env);
		// ~CgiHandler();
};

#endif