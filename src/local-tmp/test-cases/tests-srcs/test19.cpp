#include 	"../includeme.hpp"

/*
	test multi cgi
*/

std::string test19()
{
	HttpRequest req;
	std::map<std::string, std::string> headers;
	headers["host"] = "127.0.0.1";
	req.setMethod("GET");
	req.setPath("/duplicateHeaders.py");
	req.setVersion("HTTP/1.1");
	req.setHeaders(headers);
	int r_val = -1;
	int n = 1;
	char buffer[128];
	std::stringstream ss;
	std::string config_file = "test-cases/tests-conf/test17.conf";
	ConfigManager config_manager(config_file);
	if (!config_manager.load())
		return "";
ClientInfos clt;clt.clientAddr = "0.0.0.0";clt.port="8000";
	ResponseHandler testObj(clt, *config_manager.getServers().begin());	testObj.Run(req);
	wait(&r_val);
	if (testObj.RequireCgi())
	{
		ss << "CGI CHILD EXIT STATUS = " << r_val << '\n';
		while (n > 0 && !testObj.ReachedCgiBodyPhase()){//passing header buffers to cgi
			bzero(buffer, 128);
			n = read(testObj.GetCgiOutPipe().getReadFd(), buffer, 127);
			try {testObj.AppendCgiOutput(buffer);}
			catch (std::exception &ex){
				ss << "Erro: an exception is throw internally (cgi output parser)\n";
				ss << ex.what() << '\n';
				return ss.str();
			}
		}
		//creating the tmp file
		try{testObj.SetTargetFileForCgi(1);}
		catch (std::exception &ex){
			ss << "caught an exception while creating target file\n";
			ss << "errmsg :" << ex.what() << '\n';
			return ss.str();}
		while (n > 0){//reading rest of pipe to determine true body size
			bzero(buffer, 128);
			n = read(testObj.GetCgiOutPipe().getReadFd(), buffer, 127);
			try {testObj.AppendBufferToTmpFile(buffer);}
			catch (std::exception &ex){
				ss << "caught an exception while writing a buffer to tmp file\n";
				ss << "Error msg: " << ex.what();
				return ss.str();
			}
		}
		bzero(buffer, 128);
		try {testObj.FinishCgiResponse();}
		catch (std::exception &ex){
			std::cout << "exception thrown in finish function\n" << ex.what() << '\n';
			return ss.str();
		}
		ss << "=== RESPONSE HEADER ===" << '\n';
		ss << testObj.GetResponseHeader();
		ss << "=== RESPONSE BODY (from tmpfile) ===\n";
		testObj.GetTargetFilePtr()->seekg(0);
		while (testObj.GetTargetFilePtr()->good())
		{
			testObj.GetTargetFilePtr()->read(buffer, 127);
			ss << buffer;
			bzero(buffer, 128);
		}
		std::remove((TMP_FILE_PREFIX + NumtoString(1)).c_str());
	}
	else
	{
		ss << "ERROR CGI is not used!!\n";  
		ss << "=== RESPONSE HEADER ===" << '\n';
		ss << testObj.GetResponseHeader() << '\n';
	}
	return ss.str();
}

