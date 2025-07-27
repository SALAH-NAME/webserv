#include 	"../includeme.hpp"

std::string test7()
{
	/*
		normal Post request, will create the file in write only mode 
	*/
	std::stringstream ss;
	HttpRequest req;
	req.setMethod("POST");
	req.setPath("/from_client.txt");
	req.setVersion("HTTP/1.1");
		std::map<std::string, std::string> headers;
	headers["Host"] = "127.0.0.1";
	req.setHeaders(headers);
	std::string config_file = "test-cases/tests-conf/test7.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
	ServerConfig srv_conf = *config_manager.getServers().begin();
	ResponseHandler testObj("0.0.0.0", srv_conf);
	testObj.Run(req);
	ss << "=== RESPONSE HEADER ===" << '\n';
	ss << testObj.GetResponseHeader() << '\n';
	if (!testObj.GetTargetFilePtr() && testObj.GetTargetFilePtr()->is_open())
		ss << "ERROR: target file is not open" << std::endl;
	else if (access(testObj.GetResourcePath().c_str(), W_OK) != 0) 
		ss << "ERROR: target file is open but can't be used for writing" << std::endl;
	else
	{
		ss << "target File: '" + testObj.GetResourcePath() + "'\nis ready for input" <<std::endl;
		if (testObj.GetResourcePath() == "/home/midbella/Desktop/webserv/src/local-tmp/www///from_client.txt")
			std::remove(testObj.GetResourcePath().c_str());
	}
	return ss.str();
}

