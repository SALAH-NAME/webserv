#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
	content_length = -1;
	child_pid = 0;
	exec_t0 = -1;
	Body_phase = false;
}

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

void	CgiHandler::SetCgiEnvironment(HttpRequest	&http_req, const ServerConfig &conf,
			const std::string &remote_address)//adding data fetched from the request into the env object
{
	std::map<std::string, std::string> http_headers = http_req.getHeaders(); 
	env.Add("GATEWAY_INTERFACE", "CGI/1.1");
	env.Add("REQUEST_METHOD", http_req.getMethod());
	env.Add("SCRIPT_NAME", http_req.getPath());
	env.Add("SERVER_NAME", conf.getHost());
	env.Add("SERVER_PORT", NumtoString(conf.getListens()[0]));//this needs to be changed
	env.Add("SERVER_PROTOCOL", "HTTP/1.1");
	env.Add("SERVER_SOFTWARE", "Ed Edd n Eddy/1.0");	
	if (http_headers.find("content-length") != http_headers.end())
		env.Add("CONTENT_LENGTH", http_headers["content-length"]); 
	if (http_headers.find("content-yype") != http_headers.end())
		env.Add("CONTENT_TYPE", http_headers["content-type"]);
	env.Add("QUERY_STRING", http_req.getQueryString());
	env.Add("PATH_INFO", http_req.getPathInfo());
	env.Add("REMOTE_ADDR", remote_address);
	for (std::map<std::string, std::string>::iterator it = http_headers.begin(); it != http_headers.end(); it++)
		if (it->first != "content-type" && it->first != "content-length")
			env.Add("HTTP_" + it->first , it->second);//HTTP_contetn
}

void	SetCgiChildArguments(char **Argv, const std::string &interpiter, const std::string &script_path)
{
	Argv[0] = new char[interpiter.size() + 1];
	Argv[1] = new char[script_path.size() + 1];
	Argv[2] = NULL;
	std::strcpy(Argv[0], interpiter.c_str());
	std::strcpy(Argv[1], script_path.c_str());
}

pid_t	CgiHandler::GetChildPid(){return child_pid;}

Pipe&	CgiHandler::GetOutPipe(){return output_pipe;}

Pipe&	CgiHandler::GetInPipe(){return input_pipe;}

bool	CgiHandler::ReachedBodyPhase(){return Body_phase;}

std::map<std::string, std::string>&	CgiHandler::GetOutputHeaders(){return output_headers;}

std::time_t	CgiHandler::GetExecutionStartTime(){return exec_t0;}

std::string	CgiHandler::GetPreservedBody(){return preserved_body;}

int	CgiHandler::GetStatusCode(){return status_code;}

int CgiHandler::GetContentLength(){return content_length;}

std::string	CgiHandler::GetReasonPhrase(){return status_reason_phrase;}

void CgiHandler::KillChild()
{
	int status;

	ClearData();
	if (exec_t0 == -1 || waitpid(child_pid, &status, WNOHANG) == child_pid)
		return;
	kill(child_pid, SIGKILL);
}

void CgiHandler::ClearData()
{
	output_pipe.closeRead();
	output_pipe.closeWrite();
	input_pipe.closeRead();
	input_pipe.closeWrite();
	env.clear();
	child_pid = 0;
	Body_phase = false;
	exec_t0 = -1;
	status_code = 0;
	output_headers.clear();
	parsed_bytes_count = 0;
	extra_cookie_values.clear();
	status_reason_phrase.clear();
	preserved_body.clear();
	key_holder.clear();
	value_holder.clear();
}

void CgiHandler::RunCgi(HttpRequest &current_req, const ServerConfig &conf,
				const LocationConfig &cgi_conf, std::string &script_path,
					const std::string &remote_address)
{
	int 	id;
	char	**argv = new char*[3];

	ClearData();
	is_POST = current_req.getMethod() == "POST" ? true : false;
	SetCgiChildArguments(argv, cgi_conf.getCgiPass(), script_path);
	output_pipe.create();
	if (this->is_POST)
		input_pipe.create();
	SetCgiEnvironment(current_req, conf, remote_address);
	id = fork();
	if (id == -1)
		throw (std::runtime_error("failed to spawn child"));
	if (id == 0)
	{
		if (chdir(GetFileDirectoryPath(script_path).c_str()) != 0){
			std::exit(1);
			delete_strings(argv);
		}
		SetCgiChildFileDescriptors();
		execve(cgi_conf.getCgiPass().c_str(), argv, this->env.GetRawEnv());
		delete_strings(argv);
		std::exit(1);
	}
	this->exec_t0 = std::time(NULL);
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

CgiHandler::BadCgiOutput::BadCgiOutput(const std::string &err_msg){
	error = err_msg;
}

const char *CgiHandler::BadCgiOutput::what() throw(){	
	return(error.c_str());
}

CgiHandler::BadCgiOutput::~BadCgiOutput() throw(){}

