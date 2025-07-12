#include 	"includeme.hpp"

std::string test4()
{
	/*
		GET with path '/visuals' (should redir to /visuals/)
	*/
	std::stringstream ss;
	HttpRequest req;
	req.setMethod("GET");
	req.setPath("/visuals");
	req.setVersion("HTTP/1.1");
	req.getHeaders()["Host"] = "127.0.0.1";
	std::string config_file = "test-cases/tests-conf/test4.conf";
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

