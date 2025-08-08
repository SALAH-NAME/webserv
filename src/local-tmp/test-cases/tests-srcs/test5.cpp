#include 	"../includeme.hpp"

std::string test5()
{
	/*
		 URL /kapouet
			is rooted to /home/midbella/Desktop/webserv/src/local-tmp/www/,
			URL /kapouet/pouic/toto/pouet will search for
			/home/midbella/Desktop/webserv/src/local-tmp/www/pouic/toto/pouet
	*/
	std::stringstream ss;
	HttpRequest req;
	req.setMethod("GET");
	req.setPath("/kapouet/pouic/toto/pouet/");
	req.setVersion("HTTP/1.1");
		std::map<std::string, std::string> headers;
	headers["host"] = "127.0.0.1";
	req.setHeaders(headers);
	std::string config_file = "test-cases/tests-conf/test5.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
	ServerConfig srv_conf = *config_manager.getServers().begin();
	ClientInfos clt;clt.clientAddr = "0.0.0.0";clt.port="8000";
	ResponseHandler testObj(clt, *config_manager.getServers().begin());;
	testObj.Run(req);
	ss << "=== RESPONSE HEADER ===" << '\n';
	ss << testObj.GetResponseHeader() << '\n';
	if (testObj.GetTargetFilePtr() && testObj.GetTargetFilePtr()->is_open())
		ss << "ERROR: target file should not be open" << std::endl;
	else
		ss << testObj.GetResponseBody() << std::endl;
	return ss.str();
}

