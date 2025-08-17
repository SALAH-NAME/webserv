
#include "ServerManager.hpp"

void	ServerManager::handleKeepAlive(Client& client) {

	// std::cout << "   ###################### Is keep alive: " << client.getResponseHandler()->KeepConnectioAlive() << " ##############\n";
	if (client.getResponseHandler()->KeepConnectioAlive()) {
		int clientFD = client.getSocket().getFd();
		try {
			modifyEpollEvents(client.getEpfd(), clientFD, (EPOLLIN | EPOLLHUP | EPOLLERR));
		} catch (std::exception& e) {
			closeConnection(client);
		}
		client.resetAttributes();
	}
	else
		closeConnection(client);
	// std::cout << " ====>> Sent Response Seccessfully\n";
}

void	ServerManager::transmitResponseHeader(Client& client) {

	if (!client.getIsOutputAvailable())
		return ; // socket is not available "!EPOLLOUT"

	std::string& response = client.getResponseHolder();

	int bytesToSendNow =  client.getBytesToSendNow();
	// std::cout << "Bytes to send now: " << bytesToSendNow << "\n";

	size_t sentBytes;
	sentBytes = client.getSocket().send(response.c_str(), bytesToSendNow, MSG_NOSIGNAL);
	if (sentBytes <= 0)
		return ;
	// std::cout << "Sent bytes: " << sentBytes << "\n";
	client.resetLastConnectionTime();
	client.updateHeaderStateAfterSend(bytesToSendNow);
}

void	ServerManager::transmitFileResponse(Client& client) {
	if (client.getIsResponseBodySendable() == NOT_SENDABLE) {
		client.readFileBody();
	}
	else if (client.getIsResponseBodySendable() == SENDABLE) {
		client.sendFileBody();
	}
}

void    ServerManager::sendClientsResponse(int i) {

	std::map<int, Client>::iterator it = _clients.find(_events[i].data.fd);
	if (it == _clients.end())
		return ;
	Client& client = it->second;

	if (client.getResponseHeaderFlag() == RESPONSE_HEADER_READY ||
		client.getFullResponseFlag() == FULL_RESPONSE_READY)
			transmitResponseHeader(client); // send Response header to client
	else if (client.getResponseBodyFlag() == RESPONSE_BODY_READY)
		transmitFileResponse(client); // read Response body from target file and send it to client

	if (client.getResponseSent() == SENT)
		handleKeepAlive(client);
	eraseMarked();
}
