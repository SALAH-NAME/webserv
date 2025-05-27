#include "../../include/CgiHandler.hpp"

std::string num_to_string(int num){
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

void	prepare_cgi_env(Request	&http_req, Environment &my_env)//adding data fetched from the request into the env object
{
	my_env.Add("GATEWAY_INTERFACE=", "CGI/1.1");
	my_env.Add("REQUEST_METHOD=", http_req.method);
	my_env.Add("SCRIPT_NAME=", http_req.script);
	my_env.Add("SERVER_NAME=", http_req.server_name);
	my_env.Add("SERVER_PORT=", num_to_string(http_req.server_port));
	my_env.Add("SERVER_PROTOCOL=", "HTTP/1.1");
	my_env.Add("SERVER_SOFTWARE=", "Ed Edd n Eddy/1.0");
	my_env.Add("CONTENT_LENGTH=", "non");
	my_env.Add("CONTENT_TYPE=", "non");
	my_env.Add("QUERY_STRING=", http_req.query_string);
	my_env.Add("PATH_INFO=", http_req.path_info);
	my_env.Add("REMOTE_ADDR=", http_req.client_addrs);
	for (std::vector<std::pair<std::string, std::string> >::iterator it = http_req.headers.begin(); it != http_req.headers.end();it++)
	my_env.Add("HTTP_" + it->first+"=", it->second);
}

void	setArgv(char **Argv, std::string &interpiter, std::string &script_path)
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

int	CgiHandler::GetChildPid(){return child_pid;}

int	*CgiHandler::GetOutPipe(){return output_pipe;}

int	*CgiHandler::GetInPipe(){return input_pipe;}

void CgiHandler::RunCgi(Request &current_req)
{
	int 	id;
	char	**argv = new char*[3];
	
	is_POST = current_req.method == "POST" ? true : false;
	setArgv(argv, current_req.interpiter, current_req.script);
	if (pipe(output_pipe) == -1)
	throw "pipe syscall failed";
	if (this->is_POST && pipe(input_pipe) == -1)
	throw "pipe syscall failed";
	id = fork();
	if (id == 0)//child
	{
		set_fds(this->is_POST, input_pipe, output_pipe);
		prepare_cgi_env(current_req, this->env);
		execve(current_req.interpiter.c_str(), argv, this->env.GetRawEnv());
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

#include "../../include/CgiHandler.hpp"
// CgiHandler

std::string num_to_string(int num){
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

void	prepare_cgi_env(Request	&http_req, Environment &my_env)//adding data fetched from the request into the env object
{
	my_env.Add("GATEWAY_INTERFACE=", "CGI/1.1");
	my_env.Add("REQUEST_METHOD=", http_req.method);
	my_env.Add("SCRIPT_NAME=", http_req.script);
	my_env.Add("SERVER_NAME=", http_req.server_name);
	my_env.Add("SERVER_PORT=", num_to_string(http_req.server_port));
	my_env.Add("SERVER_PROTOCOL=", "HTTP/1.1");
	my_env.Add("SERVER_SOFTWARE=", "Ed Edd n Eddy/1.0");
	my_env.Add("CONTENT_LENGTH=", "non");
	my_env.Add("CONTENT_TYPE=", "non");
	my_env.Add("QUERY_STRING=", http_req.query_string);
	my_env.Add("PATH_INFO=", http_req.path_info);
	my_env.Add("REMOTE_ADDR=", http_req.client_addrs);
	for (std::vector<std::pair<std::string, std::string> >::iterator it = http_req.headers.begin(); it != http_req.headers.end();it++)
	my_env.Add("HTTP_" + it->first+"=", it->second);
}

void	setArgv(char **Argv, std::string &interpiter, std::string &script_path)
{
	Argv[0] = new char[interpiter.size()];
	Argv[1] = new char[script_path.size()];
	Argv[2] = NULL;
	std::strcpy(Argv[0], interpiter.c_str());
	std::strcpy(Argv[1], script_path.c_str());
}

CgiHandler::CgiHandler(Request &http_req) : req(http_req)
{
	is_POST = http_req.method == "POST" ? true : false;
	output_pipe = new int[2];
	input_pipe = new int[2];
	child_pid = 0;
	exec_t0 = -1;
}

int	CgiHandler::GetChildPid(){return child_pid;}

int	*CgiHandler::GetOutPipe(){return output_pipe;}

int	*CgiHandler::GetInPipe(){return input_pipe;}

void CgiHandler::RunCgi()
{
	int 	id;
	char	**argv = new char*[3];
	
	setArgv(argv, req.interpiter, req.script);
	if (pipe(output_pipe) == -1)
		throw "pipe syscall failed";
	if (this->is_POST && pipe(input_pipe) == -1)
		throw "pipe syscall failed";
	id = fork();
	if (id == 0)//child
	{
		set_fds(this->is_POST, input_pipe, output_pipe);
		prepare_cgi_env(this->req, this->env);
		execve(this->req.interpiter.c_str(), argv, this->env.GetRawEnv());
		throw "failed to spawn child";
	}
	close(output_pipe[1]);
	close(input_pipe[0]);
	delete_strings(argv);
	this->exec_t0 = time(NULL);
	this->child_pid = id;
}

CgiHandler::~CgiHandler()
{
	delete[] output_pipe;
	delete[] input_pipe;
}
