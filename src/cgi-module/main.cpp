#include "../../include/Environment.hpp"
#include <string.h>
#include <stdio.h>
#include "../../include/Request.hpp"
#include "../../include/CgiHandler.hpp"

using namespace std;
	
void test_req_init(Request &req)
{
	req.method = "POST";
	req.script = "./post_test.py";
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

int main()
{
	Request req;
	Environment test;
	test_req_init(req);
	CgiHandler tst;
	tst.RunCgi(req);
	char c;
	if (req.method == "POST"){//simulation of the content of a post body, originally this data should be sent in the multiplexing part (the fd should be added to epoll)
		for (int i =0;i<5;i++)
			write(tst.GetInPipe()[1], "bla bla\n", 8);
		close(tst.GetInPipe()[1]);
	}
	while(read(tst.GetOutPipe()[0], &c, 1))//same here if we want to correctly read the output of the cgi this fd also should be added to epoll and get monitored
		write(1, &c, 1);
}
 