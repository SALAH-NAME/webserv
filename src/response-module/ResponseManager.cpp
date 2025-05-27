#include "ResponseManager.hpp"

ResponseManager::ResponseManager(void) {
}

ResponseManager::~ResponseManager(void) {}

void	ResponseManager::createResponse(Request _parsedRequest) {
	Responses.push_back(Response(_parsedRequest));
}
