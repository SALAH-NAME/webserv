#include "Cgi.hpp"

using namespace std;

string num_to_string(int num){
	stringstream ss;
	ss << num;
	return ss.str();
}

void	prepare_cgi_env(Request	&http_req, Environment &my_env)
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
	for (vector<pair<string, string> >::iterator it = http_req.headers.begin(); it != http_req.headers.end();it++)
		my_env.Add("HTTP_" + it->first+"=", it->second);
}

CgiHandler::CgiHandler(Request http_req, Environment my_env)
{
	int id;
	char *tab[3];
	tab[0] = (char *)"python3";
	tab[1] = (char *)http_req.script.c_str();
	tab[2] = NULL;
	cout << "argv[0] = "<< tab[0] << endl;
	cout << "cmd path =" << http_req.interpiter<< endl;
	id = fork();
	if (id != 0)//child
	{
		prepare_cgi_env(http_req, my_env);
		execve(http_req.interpiter.c_str(), tab, my_env.GetRawEnv());
		cerr << "execve failed\n";
	}
}
