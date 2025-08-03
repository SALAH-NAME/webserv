/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SendResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 09:39:10 by karim             #+#    #+#             */
/*   Updated: 2025/08/03 18:52:16 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>

void	ServerManager::transmitResponseHeader(Client& client, int serverIndex) {

	std::string& response = client.getResponseHolder();

	int bytesToSendNow =  client.getBytesToSendNow();

	size_t sentBytes;
	try {
		sentBytes = client.getSocket().send(response.c_str(), bytesToSendNow, MSG_NOSIGNAL);
		if (sentBytes > 0) {	
			client.resetLastConnectionTime();
			if (client.updateHeaderStateAfterSend(bytesToSendNow))
				_servers[serverIndex].closeConnection(client.getSocket().getFd());
		}
		else
			throwIfSocketError("send()");
	} catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
	}
}

void	ServerManager::transmitFileResponse(Client& client, int serverIndex) {
	try {
		if (client.getIsResponseBodySendable() == NOT_SENDABLE) {
			if (client.readFileBody())
				_servers[serverIndex].closeConnection(client.getSocket().getFd());
		}
		else if (client.getIsResponseBodySendable() == SENDABLE) {
			if (client.sendFileBody())
				_servers[serverIndex].closeConnection(client.getSocket().getFd());
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
	}
	_servers[serverIndex].eraseMarked();
}
