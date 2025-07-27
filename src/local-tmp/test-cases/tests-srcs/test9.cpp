#include 	"../includeme.hpp"

std::string test9()
{
	/*
		checking DELETE request
	*/
	std::stringstream ss;
	HttpRequest req;
	std::string temp_file = "/home/midbella/Desktop/webserv/src/local-tmp/www/pouic/toto/delete_me.txt";
	req.setMethod("DELETE");
	if (system(("touch " + temp_file).c_str()) != 0)
	{
		ss << "Error couldn't create a tmp file" << std::endl;
		return ss.str();
	}
	else
		ss << "Created a tmp file:\n" + temp_file << std::endl; 
	// system(("chmod 0 " + temp_file).c_str()); // to change the permissions
	req.setPath("/kapouet/pouic/toto/delete_me.txt");
	req.setVersion("HTTP/1.1");
		std::map<std::string, std::string> headers;
	headers["Host"] = "127.0.0.1";
	req.setHeaders(headers);
	std::string config_file = "test-cases/tests-conf/test9.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
	ServerConfig srv_conf = *config_manager.getServers().begin();
	ResponseHandler testObj("0.0.0.0", srv_conf);
	testObj.Run(req);
	ss << "=== RESPONSE HEADER ===" << '\n';
	ss << testObj.GetResponseHeader() << '\n';
	if (access(temp_file.c_str(), F_OK) == 0)
		ss << "Error the file still exists";
	else
		ss << "Successfully deleted the file" << std::endl;
	return ss.str();
}

