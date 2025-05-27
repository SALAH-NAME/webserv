// #include "Response.hpp"
// #include "Request.hpp"

#include "ResponseManager.hpp"

void    print_line(std::string line) {
	for (int i = 0; i < line.size(); i++) {
		if (line[i] == '\r')
			std::cout << "\\r";
		else if (line[i] == '\n')
			std::cout << "\\n\n";
		else
			std::cout << line[i];
	}
}


void	RequestGenerator(Request &req) {
	req.set_up("localhost", "GET", "/test.html", "HTTP/1.1", "close");
}

int main() {
	Request	req;

	RequestGenerator(req);

	req.printer();
	std::cout << "=========================\n\n";

	ResponseManager Manager;
	Manager.createResponse(req);
	Manager.buildHttpResponse();

	std::cout << std::endl;
}