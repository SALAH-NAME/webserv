#include 	"../includeme.hpp"

/*
	simple routing in case of cgi request
*/



void init(HttpRequest &req)
{
	std::map<std::string, std::string> headers;
	headers["host"] = "127.0.0.1";
	req.setMethod("GET");
	req.setPath("/PrintEnv.py");
	req.setVersion("HTTP/1.1");
	req.setHeaders(headers);
}
typedef const std::map<std::string, LocationConfig> LOCATIONS;

std::string test13()
{
	int r_val = -1;
	int n = 1;
	char buffer[128];
	std::stringstream ss;
	HttpRequest req;
	std::string config_file = "test-cases/tests-conf/test13.conf";
	ConfigManager config_manager(config_file);
	init(req);
	if (!config_manager.load())
		return "";
	ResponseHandler testObj("0.0.0.0", *config_manager.getServers().begin());
	testObj.Run(req);
	wait(&r_val);

	if (testObj.RequireCgi())
	{
		ss << "CGI CHILD EXIT STATUS = " << r_val << '\n';
		ss << "=== CGI RAW OUTPUT ===\n";
		while (n > 0)
		{
			bzero(buffer, 128);
			n = read(testObj.GetCgiOutPipe().getReadFd(), buffer, 127);
			ss << buffer;
		}
	}
	else
	{
		ss << "ERROR CGI is not used!!\n";
		ss << "=== RESPONSE HEADER ===" << '\n';
		ss << testObj.GetResponseHeader() << '\n';
	}
	return ss.str();
}

