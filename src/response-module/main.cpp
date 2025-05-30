#include "../../include/Request.hpp"

void test_req_init(Request &req)
{
	req.method = "GET";
	req.path = "/";
	req.http_version = "HTTP/1.1";
}

int main()
{

}
