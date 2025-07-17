#include "CgiHandler.hpp"

std::string NumtoString(int num){
	std::stringstream ss;
	ss << num;
	return ss.str();
}

void	CgiHandler::SetCgiChildFileDescriptors()
{
	output_pipe.closeRead();
	dup2(output_pipe.getWriteFd(), 1);
	if (is_POST){
		input_pipe.closeWrite();
		dup2(input_pipe.getReadFd(), 0);
	}
}

void	CgiHandler::SetCgiEnvironment(HttpRequest	&http_req, const ServerConfig &conf)//adding data fetched from the request into the env object
{
	env.Add("GATEWAY_INTERFACE=", "CGI/1.1");
	env.Add("REQUEST_METHOD=", http_req.getMethod());
	env.Add("SCRIPT_NAME=", http_req.getPath());
	env.Add("SERVER_NAME=", conf.getSessionName());
	env.Add("SERVER_PORT=", NumtoString(conf.getListen()));
	env.Add("SERVER_PROTOCOL=", "HTTP/1.1");
	env.Add("SERVER_SOFTWARE=", "Ed Edd n Eddy/1.0");
	env.Add("CONTENT_LENGTH=", "non");
	env.Add("CONTENT_TYPE=", "non");
	env.Add("QUERY_STRING=", http_req.getQueryString());
	env.Add("PATH_INFO=", http_req.getPathInfo());
	//env.Add("REMOTE_ADDR=", http_req.getClientAddrs()); still need to add client ip addrs to the environment
	for (std::map<std::string, std::string>::iterator it = http_req.getHeaders().begin(); it != http_req.getHeaders().end(); it++)
		env.Add("HTTP_" + it->first + "=", it->second);
}

void	SetCgiChildArguments(char **Argv, const std::string &interpiter, const std::string &script_path)
{
	Argv[0] = new char[interpiter.size()];
	Argv[1] = new char[script_path.size()];
	Argv[2] = NULL;
	std::strcpy(Argv[0], interpiter.c_str());
	std::strcpy(Argv[1], script_path.c_str());
}

CgiHandler::CgiHandler()
{
	child_pid = 0;
	exec_t0 = -1;
}

pid_t	CgiHandler::GetChildPid(){return child_pid;}

Pipe&	CgiHandler::GetOutPipe(){return output_pipe;}

Pipe&	CgiHandler::GetInPipe(){return input_pipe;}

void CgiHandler::RunCgi(HttpRequest &current_req, const ServerConfig &conf,
				const LocationConfig &cgi_conf, std::string &script_path)
{
	int 	id;
	char	**argv = new char*[3];
	
	is_POST = current_req.getMethod() == "POST" ? true : false;
	SetCgiChildArguments(argv, cgi_conf.getCgiPass(), script_path);
	output_pipe.create();
	if (this->is_POST)
		input_pipe.create();
	id = fork();
	if (id == -1)
		throw (std::runtime_error("failed to spawn child"));
	if (id == 0)//child
	{
		SetCgiChildFileDescriptors();
		SetCgiEnvironment(current_req, conf);
		execve(cgi_conf.getCgiPass().c_str(), argv, this->env.GetRawEnv());
		_exit(1);
	}
	this->exec_t0 = time(NULL);
	this->child_pid = id;
	output_pipe.closeWrite();
	input_pipe.closeRead();
	delete_strings(argv);
	env.clear();
}

CgiHandler::~CgiHandler()
{
	output_pipe.closeRead();
	input_pipe.closeWrite();
	env.clear();
}
