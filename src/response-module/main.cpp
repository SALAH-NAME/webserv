#include "../../include/Request.hpp"

void test_req_init(Request &req)
{
	req.set_method("GET");
	req.set_http_version("HTTP/1.1");
	req.set_path("/");
}

int main()
{

}
