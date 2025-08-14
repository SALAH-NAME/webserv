#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
	content_length = -1;
	child_pid = 0;
	prev_buf_l_char = 0;
	exec_t0 = -1;
	is_POST = false;
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
	dup2(output_pipe.getWriteFd(), STDOUT_FILENO);
	if (is_POST){
		input_pipe.closeWrite();
		dup2(input_pipe.getReadFd(), STDIN_FILENO);
	}
	else
		close(STDIN_FILENO);
}

void	CgiHandler::SetCgiEnvironment(HttpRequest	&http_req, const ServerConfig &conf,
			ClientInfos &client_info)//adding data fetched from the request into the env object
{
	std::map<std::string, std::string> http_headers = http_req.getHeaders(); 
	env.Add("GATEWAY_INTERFACE", "CGI/1.1");
	env.Add("REQUEST_METHOD", http_req.getMethod());
	env.Add("SCRIPT_NAME", http_req.getPath());
	env.Add("SERVER_NAME", conf.getHost());
	env.Add("SERVER_PORT", client_info.port);//
	env.Add("SERVER_PROTOCOL", http_req.getVersion());
	env.Add("SERVER_SOFTWARE", "Ed Edd n Eddy/1.0");	
	if (http_headers.find("content-length") != http_headers.end())
		env.Add("CONTENT_LENGTH", http_headers["content-length"]); 
	if (http_headers.find("content-type") != http_headers.end())
		env.Add("CONTENT_TYPE", http_headers["content-type"]);
	env.Add("QUERY_STRING", http_req.getQueryString());
	env.Add("PATH_INFO", http_req.getPathInfo());
	env.Add("REMOTE_ADDR", client_info.clientAddr);
	for (std::map<std::string, std::string>::iterator it = http_headers.begin(); it != http_headers.end(); it++)
		if (it->first != "content-type" && it->first != "content-length")
			env.Add("HTTP_" + it->first , it->second);
}

void	SetCgiChildArguments(char **Argv, const std::string &interpiter, const std::string &script_path)
{
	std::string script_name;

	int i = script_path.size() - 1;
	for (; i >= 0; i--)
		if (script_path[i] == '/')
			break;
	script_name = script_path.substr(i + 1);
	Argv[0] = new char[interpiter.size() + 1];
	Argv[1] = new char[script_name.size() + 1];
	Argv[2] = NULL;
	std::strcpy(Argv[0], interpiter.c_str());
	std::strcpy(Argv[1], script_name.c_str());
}

pid_t	CgiHandler::GetChildPid(){return child_pid;}

Pipe&	CgiHandler::GetOutPipe(){return output_pipe;}

Pipe&	CgiHandler::GetInPipe(){return input_pipe;}

bool	CgiHandler::ReachedBodyPhase(){return Body_phase;}

std::map<std::string, std::string>&	CgiHandler::GetOutputHeaders(){return output_headers;}

std::time_t	CgiHandler::GetExecutionStartTime(){return exec_t0;}

std::string	CgiHandler::GetPreservedBody(){return preserved_body;}

bool CgiHandler::PostReq()
{
	return is_POST;
}

int	CgiHandler::GetStatusCode(){return status_code;}

int CgiHandler::GetContentLength(){return content_length;}

std::string	CgiHandler::GetReasonPhrase(){return status_reason_phrase;}

std::vector<std::string> &CgiHandler::GetExtraCookieValues(){return extra_cookie_values;}

void CgiHandler::KillChild()
{
	ClearData();
	if (waitpid(child_pid, NULL, WNOHANG) == child_pid)
		return;
	kill(child_pid, SIGKILL);
	child_pid = -1;
}

void CgiHandler::ClearData()
{
	is_POST = false;
	output_pipe.closeRead();
	output_pipe.closeWrite();
	input_pipe.closeRead();
	input_pipe.closeWrite();
	env.clear();
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
	prev_buf_l_char = 0;
}

void CgiHandler::RunCgi(HttpRequest &current_req, const ServerConfig &conf,
				const LocationConfig &cgi_conf, std::string &script_path,
					ClientInfos &client_info)
{
	// std::cout << "called run cgi" << std::endl;//logger
	int 	id;
	char	**argv = new char*[3];

	ClearData();
	child_pid = -1;
	is_POST = current_req.getMethod() == "POST" ? true : false;
	SetCgiChildArguments(argv, cgi_conf.getCgiPass(), script_path);
	output_pipe.create();
	if (this->is_POST)
		input_pipe.create();
	SetCgiEnvironment(current_req, conf, client_info);
// 	std::cout << "child args, env vars and pipes are created" << std::endl;//logger
	id = fork();
	if (id == -1)
		throw (std::runtime_error("failed to spawn child"));
	if (id == 0)
	{
		// std::cout << "inside the spawned child code" << std::endl;//logger
		if (chdir(GetFileDirectoryPath(script_path).c_str()) != 0){
			delete_strings(argv);
			std::exit(1); // Fixed: free memory before exit
		}
		SetCgiChildFileDescriptors();
		execve(cgi_conf.getCgiPass().c_str(), argv, this->env.GetRawEnv());
		delete_strings(argv);
		std::cout << "execve failed" << std::endl;//logger
		std::exit(1);
	}
	this->exec_t0 = std::time(NULL);
	this->child_pid = id;
	output_pipe.closeWrite();
	input_pipe.closeRead();
	delete_strings(argv);
	env.clear();
	// std::cout << "a CGI child is running in the back ground" << std::endl;//logger
}

CgiHandler::~CgiHandler()
{
	output_pipe.closeRead();
	input_pipe.closeWrite();
	env.clear();
}

CgiHandler::BadCgiOutput::BadCgiOutput(const std::string &err_msg){
// 	std::cout << "-------------cgi handler internal exception------------" << std::endl;//logger
// 	std::cout << err_msg << std::endl;//logger
	error = err_msg;
}

const char *CgiHandler::BadCgiOutput::what() throw(){	
	return(error.c_str());
}

CgiHandler::BadCgiOutput::~BadCgiOutput() throw(){}

