/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SendResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 09:39:10 by karim             #+#    #+#             */
/*   Updated: 2025/07/21 22:23:08 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static void	finalizeRequestHandling(Server& server, Client& client, int sentBytes) {
	client.setSentBytes(sentBytes);
	if (client.getSentBytes() == client.getResponseSize()){
		client.clearRequestHolder();
		client.resetSendBytes();
		client.setResponseInFlight(false);
		if (!client.getIsKeepAlive()) {
			server.closeConnection(client.getSocket().getFd());
		}
		// std::cout << "    ===> sent response to " << client.getSocket().getFd() << " <<=== \n";
		server.closeConnection(client.getSocket().getFd());
	}
}

void	ServerManager::transmitResponse(Client& client, int serverIndex) {
	std::string& response = client.getResponseHolder();

	// printRequestAndResponse("Resposne", response);

	int bytesToSendNow =  client.getBytesToSendNow();
	ssize_t sentBytes;
	try {
		sentBytes = client.getSocket().send(response.c_str() + client.getSentBytes(), bytesToSendNow);
		// std::cout << "sent bytes ==> " << sentBytes << "\n";
		if (sentBytes > 0)
			finalizeRequestHandling(_servers[serverIndex], client, sentBytes);
		else
			throwIfSocketError("send()");
	} catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
	}
}

void    ServerManager::sendClientsResponse(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (it->second.getResponseInFlight()) {
			transmitResponse(it->second, serverIndex);
		}
	}
	_servers[serverIndex].eraseMarked();
}