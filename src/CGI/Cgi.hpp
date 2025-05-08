#ifndef CGI_HPP
#define CGI_HPP

#include "Request.tmp.hpp"
#include "Environment.hpp"

class CgiHandler{
	private:
		Request req;
	public:
		CgiHandler(Request http_req, Environment my_env);
		~CgiHandler();
};

#endif