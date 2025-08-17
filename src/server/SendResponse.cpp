
#include "ServerManager.hpp"

void	ServerManager::handleKeepAlive(Client& client) {

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
}

void	ServerManager::transmitResponseHeader(Client& client) {

	if (!client.getIsOutputAvailable() == ON)
		return ;

	std::string& response = client.getResponseHolder();

	int bytesToSendNow =  client.getBytesToSendNow();

	size_t sentBytes;
	try {
		sentBytes = client.getSocket().send(response.c_str(), bytesToSendNow, MSG_NOSIGNAL);
		if (sentBytes <= 0)
			return ;
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
		return ;
	}
	client.resetLastConnectionTime();
	client.updateHeaderStateAfterSend(bytesToSendNow);
}

void	ServerManager::transmitFileResponse(Client& client) {
	if (client.getIsResponseBodySendable() == OFF) {
		client.readFileBody();
	}
	else if (client.getIsResponseBodySendable() == ON) {
		client.sendFileBody();
	}
}

void    ServerManager::sendClientsResponse(int i) {

	std::map<int, Client>::iterator it = _clients.find(_events[i].data.fd);
	if (it == _clients.end())
		return ;
	Client& client = it->second;

	if (client.getResponseHeaderFlag() == ON ||
		client.getFullResponseFlag() == ON)
			transmitResponseHeader(client);
	else if (client.getResponseBodyFlag() == ON)
		transmitFileResponse(client);

	if (client.getResponseSent() == ON)
		handleKeepAlive(client);
	eraseMarked();
}
