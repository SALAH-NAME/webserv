
#include "Server.hpp"

void	ServerManager::handleKeepAlive(Client& client, int serverIndex) {

	// std::cout << "   ###################### Is keep alive: " << client.getResponseHandler()->KeepConnectioAlive() << " ##############\n";
	
	if (client.getResponseHandler()->KeepConnectioAlive())
		client.resetAttributes();
	else
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
}

void	ServerManager::transmitResponseHeader(Client& client, int serverIndex) {

	if (!client.getIsOutputAvailable())
		return ; // socket is not available "!EPOLLOUT"

	std::string& response = client.getResponseHolder();

	int bytesToSendNow =  client.getBytesToSendNow();
	// std::cout << "Bytes to send now: " << bytesToSendNow << "\n";
	// exit(0);

	size_t sentBytes;
	try {
		sentBytes = client.getSocket().send(response.c_str(), bytesToSendNow, MSG_NOSIGNAL);
		// std::cout << "Sent bytes: " << sentBytes << "\n";
		if (sentBytes > 0) {	
			client.resetLastConnectionTime();
			client.updateHeaderStateAfterSend(bytesToSendNow);
		}
		else
			throwIfSocketError("send()");
	// printRequestAndResponse("updated response header holder", response);
	} catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
	}
}

void	ServerManager::transmitFileResponse(Client& client, int serverIndex) {

	try {
		if (client.getIsResponseBodySendable() == NOT_SENDABLE) {
			client.readFileBody();
		}
		else if (client.getIsResponseBodySendable() == SENDABLE) {
			client.sendFileBody();
		}
		
	} catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
	}
}

void    ServerManager::sendClientsResponse(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (it->second.getResponseHeaderFlag() == RESPONSE_HEADER_READY ||
		it->second.getFullResponseFlag() == FULL_RESPONSE_READY)
			transmitResponseHeader(it->second, serverIndex); // send Response header to client
		else if (it->second.getResponseBodyFlag() == RESPONSE_BODY_READY)
			transmitFileResponse(it->second, serverIndex); // read Response body from target file and send it to client

		if (it->second.getResponseSent() == SENT)
			handleKeepAlive(it->second, serverIndex);
	}
	_servers[serverIndex].eraseMarked();
}
