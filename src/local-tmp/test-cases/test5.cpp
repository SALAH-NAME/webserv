#include 	"includeme.hpp"

std::string test5()
{
	/*
		 URL /kapouet
			is rooted to /home/midbella/Desktop/webserv/src/local-tmp/www/,
			URL /kapouet/pouic/toto/pouet will search for
			/home/midbella/Desktop/webserv/src/local-tmp/www/pouic/toto/pouet
	*/
	std::stringstream ss;
	Request req;
	req.set_method("GET");
	req.set_path("/kapouet/pouic/toto/pouet/");
	req.set_http_version("HTTP/1.1");
	req.set_cgi_required(false);
	req.getHeaders()["Host"] = "127.0.0.1";
	std::string config_file = "test-cases/tests-conf/test5.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
	ServerConfig srv_conf = *config_manager.getServers().begin();
	ResponseHandler testObj(srv_conf);
	testObj.Run(req);
	ss << "=== RESPONSE HEADER ===" << '\n';
	ss << testObj.GetResponseHeader() << '\n';
	if (testObj.GetTargetFilePtr()->is_open())
		ss << "ERROR: target file should not be open" << std::endl;
	else
		ss << testObj.GetResponseBody() << std::endl;
	return ss.str();
}

