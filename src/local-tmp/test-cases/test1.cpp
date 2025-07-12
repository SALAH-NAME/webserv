#include 	"includeme.hpp"

std::string test1()
{
	/*
		GET with path '/' and index file exists
	*/
	std::stringstream ss;
	HttpRequest req;
	req.setMethod("GET");
	req.setPath("/");
	req.setVersion("HTTP/1.1");
	req.getHeaders()["Host"] = "127.0.0.1";
	std::string config_file = "test-cases/tests-conf/test1.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
	ServerConfig srv_conf = *config_manager.getServers().begin();
	ResponseHandler testObj(srv_conf);
	testObj.Run(req);
	ss << "=== RESPONSE HEADER ===" << '\n';
	ss << testObj.GetResponseHeader() << '\n';
	if (testObj.GetTargetFilePtr()->is_open())
	{
		ss << "=== FILE CONTENT ===" << std::endl;
		std::fstream* filePtr = testObj.GetTargetFilePtr();		
		filePtr->clear();
		std::string line;
		while (std::getline(*filePtr, line))
			ss << line << '\n';
	}
	else
		std::cout << "ERROR: Target file is not open!" << std::endl;
	return ss.str();
}

