#include 	"../includeme.hpp"

std::string test12()
{
	/*
		POST to a route that allows it, but the dir used has invalid permissions
	*/
	std::stringstream ss;
	HttpRequest req;
	req.setMethod("POST");
	req.setPath("/invalid-dir/file.txt");
	req.setVersion("HTTP/1.1");
	std::map<std::string, std::string> headers;
	headers["host"] = "127.0.0.1";
	req.setHeaders(headers);
	std::string config_file = "test-cases/tests-conf/test11.conf";
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

