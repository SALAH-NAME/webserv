#include "ServerManager.hpp"

void	ServerManager::generatResponses(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client& client = it->second;
		if (client.getGenerateInProcess() == GENERATE_RESPONSE_OFF)
			continue ;

		client.setResponseInFlight(true);
		client.setGenerateInProcess(GENERATE_RESPONSE_OFF);
		// client.prinfRequestinfos();exit(0);
	}

}