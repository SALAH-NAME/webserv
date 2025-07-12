#include "CgiHandler.hpp"

std::string NumtoString(int num){
	std::stringstream ss;
	ss << num;
	return ss.str();
}

void	set_fds(bool is_POST, int *in_pipe, int *out_pipe)
{
	close(out_pipe[0]);
	dup2(out_pipe[1], 1);
	if (is_POST){
		close (in_pipe[1]);
		dup2(in_pipe[0], 0);
	}
}

void	prepare_cgi_env(HttpRequest &http_req, Environment &my_env, const ServerConfig &conf)//adding data fetched from the request into the env object
{
	my_env.Add("GATEWAY_INTERFACE=", "CGI/1.1");
	my_env.Add("REQUEST_METHOD=", http_req.getMethod());
	my_env.Add("SCRIPT_NAME=", http_req.getPath());
	my_env.Add("SERVER_NAME=", conf.getSessionName());
	my_env.Add("SERVER_PORT=", NumtoString(conf.getListen()));
	my_env.Add("SERVER_PROTOCOL=", "HTTP/1.1");
	my_env.Add("SERVER_SOFTWARE=", "Ed Edd n Eddy/1.0");
	my_env.Add("CONTENT_LENGTH=", "non");
	my_env.Add("CONTENT_TYPE=", "non");
	my_env.Add("QUERY_STRING=", http_req.getQueryString());
	my_env.Add("PATH_INFO=", http_req.getPathInfo());
	//my_env.Add("REMOTE_ADDR=", http_req.getClientAddrs()); still need to add client addrs to the environment
	for (std::map<std::string, std::string>::iterator it = http_req.getHeaders().begin(); it != http_req.getHeaders().end(); it++)
		my_env.Add("HTTP_" + it->first + "=", it->second);
}

void	setArgv(char **Argv, const std::string &interpiter, const std::string &script_path)
{
	Argv[0] = new char[interpiter.size()];
	Argv[1] = new char[script_path.size()];
	Argv[2] = NULL;
	std::strcpy(Argv[0], interpiter.c_str());
	std::strcpy(Argv[1], script_path.c_str());
}

CgiHandler::CgiHandler()
{
	output_pipe = new int[2];
	input_pipe = new int[2];
	child_pid = 0;
	exec_t0 = -1;
}

pid_t	CgiHandler::GetChildPid(){return child_pid;}

int	*CgiHandler::GetOutPipe(){return output_pipe;}

int	*CgiHandler::GetInPipe(){return input_pipe;}

void CgiHandler::RunCgi(HttpRequest &current_req, const ServerConfig &conf,
				const LocationConfig &cgi_conf, std::string &script_path)
{
	int 	id;
	char	**argv = new char*[3];
	
	is_POST = current_req.getMethod() == "POST" ? true : false;
	setArgv(argv, cgi_conf.getCgiPass(), script_path);
	if (pipe(output_pipe) == -1)
	throw "pipe syscall failed";
	if (this->is_POST && pipe(input_pipe) == -1)
	throw "pipe syscall failed";
	id = fork();
	if (id == 0)//child
	{
		set_fds(this->is_POST, input_pipe, output_pipe);
		prepare_cgi_env(current_req, this->env, conf);
		execve(cgi_conf.getCgiPass().c_str(), argv, this->env.GetRawEnv());
		throw "failed to spawn child";
	}
	this->exec_t0 = time(NULL);
	this->child_pid = id;
	close(output_pipe[1]);
	close(input_pipe[0]);
	delete_strings(argv);
	env.clear();
}

CgiHandler::~CgiHandler()
{
	close(output_pipe[0]);
	close(input_pipe[1]);
	delete[] output_pipe;
	delete[] input_pipe;
	env.clear();
}
