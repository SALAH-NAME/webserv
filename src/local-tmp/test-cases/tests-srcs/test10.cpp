#include 	"../includeme.hpp"

std::string test10()
{
	/*
		GET with path '/test/www/' and an index file exists
		in the resolved route
	*/
	std::stringstream ss;
	HttpRequest req;
	req.setMethod("GET");
	req.setPath("/test/www/");
	req.setVersion("HTTP/1.1");
	std::map<std::string, std::string> headers;
	headers["host"] = "127.0.0.1";
	req.setHeaders(headers);
	std::string config_file = "test-cases/tests-conf/test1.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
	ServerConfig srv_conf = *config_manager.getServers().begin();
	ResponseHandler testObj("0.0.0.0", srv_conf);
	testObj.Run(req);
	ss << "=== RESPONSE HEADER ===" << '\n';
	ss << testObj.GetResponseHeader() << '\n';
	if (testObj.GetTargetFilePtr() && testObj.GetTargetFilePtr()->is_open())
	{
		ss << "=== FILE CONTENT ===" << std::endl;
		std::fstream* filePtr = testObj.GetTargetFilePtr();		
		filePtr->clear();
		std::string line;
		while (std::getline(*filePtr, line))
			ss << line << '\n';
	}
	else{
		ss << "ERROR: Target file is not open!" << std::endl;
		ss << testObj.GetResponseBody();
	}
	return ss.str();
}

