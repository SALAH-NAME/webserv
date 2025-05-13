#include "Cgi.hpp"

using namespace std;

CgiHandler::CgiHandler(Request http_req, Environment my_env)
{
	int id;
	id = fork();
	if (id != 0)//child
	{
		//set the env and run the chosen cgi with the script path as a pram
		
	}
}
