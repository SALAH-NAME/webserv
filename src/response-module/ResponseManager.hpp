#ifndef RESPONSEMANAGER_HPP
#define RESPONSEMANAGER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <ctime>

#include "Request.hpp"

class Response;

#define VALID_METHOD 1
#define VALID_PATH 2
#define VALID_METHOD_AND_PATH 3 // valid method and path
#define	VALID_VERSION 4
#define STATUSLINE_VALID 15 // PATH and VERSION are VALID

#define NOTEXIST 33
#define NOTALLOWED 44
#define VALID 55
#define NOTUPPER 66

class ResponseManager {
	private:
		std::vector<Response>	Responses;

	public:
		ResponseManager(void);
		~ResponseManager(void);
		void	createResponse(Request _parsedRequest);
		void	buildHttpResponse(void);

};

void	constructStatusLine(Response& res);
void    print_line(std::string line);

#include "Response.hpp"

#endif