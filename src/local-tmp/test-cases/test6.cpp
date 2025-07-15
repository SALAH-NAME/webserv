#include 	"includeme.hpp"

std::string test6()
{
	/*
		location matched to a redirection location
	*/
	std::stringstream ss;
	HttpRequest req;
	req.setMethod("GET");
	req.setPath("/old/media/");
	req.setVersion("HTTP/1.1");
		std::map<std::string, std::string> headers;
	headers["Host"] = "127.0.0.1";
	req.setHeaders(headers);
	std::string config_file = "test-cases/tests-conf/test6.conf";
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

