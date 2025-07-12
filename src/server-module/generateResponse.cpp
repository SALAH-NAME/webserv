#include "ServerManager.hpp"

void	ServerManager::generatResponses(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (it->second.getRequestIsValid() == GENERATE_RESPONSE_OFF)
			continue ;

		it->second.setResponseInFlight(true);
	}

}