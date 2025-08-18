
#include "ServerManager.hpp"
#include "SimpleLogger.hpp"

void	ServerManager::handleKeepAlive(Client& client) {
	
	// HttpRequest& req = client.getHttpRequest();
	// std::string clientIP = client.getClientInfos().clientAddr;
	// int clientPort = std::atoi(client.getClientInfos().port.c_str());
	// std::string responseHeader = client.getResponseHandler()->GetResponseHeader();
	
	// int statusCode = 200; // default
	// size_t spacePos = responseHeader.find(' ');
	// if (spacePos != std::string::npos) {
	// 	size_t nextSpacePos = responseHeader.find(' ', spacePos + 1);
	// 	if (nextSpacePos != std::string::npos) {
	// 		std::string statusStr = responseHeader.substr(spacePos + 1, nextSpacePos - spacePos - 1);
	// 		statusCode = std::atoi(statusStr.c_str());
	// 	}
	// }
	
	// LOG_RESPONSE(clientIP, clientPort, req.getMethod(), req.getUri(), req.getVersion(), statusCode);

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
		std::string clientIP = client.getClientInfos().clientAddr;
		int clientPort = std::atoi(client.getClientInfos().port.c_str());
		LOG_ERROR_CLIENT("Error sending response", clientIP, clientPort);
		LOG_ERROR_F("Send error details: {}", e.what());
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
