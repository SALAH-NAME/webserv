
#include "ResponseManager.hpp"

static void	handleStaticResponse(Response& res) {
	res.build();
	print_line(res.getResponse());
}

static void	dynamicResponse(Response& res) {
	std::cout << "'Dynamic Response' Not Yet!!\n";
	exit(0);
}


void	ResponseManager::buildHttpResponse(void) {

	for (int i = 0; i < Responses.size(); i++) {
		if (Responses[i].isStatic())
			handleStaticResponse(Responses[i]);
		else
			dynamicResponse(Responses[i]);
			
		// Responses[i].setResponseHolder();
		// print_line(Responses[i].getResponseHolder());
	}
}
