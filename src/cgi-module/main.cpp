#include "Environment.hpp"
#include <string.h>
#include <stdio.h>
#include "Request.tmp.hpp"
#include "Cgi.hpp"
using namespace std;
	
void test_req_init(Request &req)
{
	req.method = "GET";
	req.script = "./test_script.py";
	req.interpiter = "/usr/bin/python3";
	req.path_info = "non";
	req.query_string = "?key=val&key1=val2";
	req.content_length = "";
	req.content_type = "";
	req.server_name = "webserv";
	req.server_port = 8000;
	req.client_addrs = "192.168.32.12";
	req.client_port = 8000;

}

int main(int ac, char **av)
{
	Request req;
	Environment test;
	test_req_init(req);
	CgiHandler tst(req, test);
}
