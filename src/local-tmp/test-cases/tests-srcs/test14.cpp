#include 	"../includeme.hpp"

/*
	extra routing case in cgi request
*/

typedef const std::map<std::string, LocationConfig> LOCATIONS;

std::string test14()
{
	HttpRequest req;
	std::map<std::string, std::string> headers;
	headers["host"] = "127.0.0.1";
	req.setMethod("GET");
	req.setPath("/py/PrintEnv.py");
	req.setVersion("HTTP/1.1");
	req.setHeaders(headers);
	int r_val = -1;
	int n = 1;
	char buffer[128];
	std::stringstream ss;
	std::string config_file = "test-cases/tests-conf/test14.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
ClientInfos clt;clt.clientAddr = "0.0.0.0";clt.port="8000";
	ResponseHandler testObj(clt, *config_manager.getServers().begin());	testObj.Run(req);
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

