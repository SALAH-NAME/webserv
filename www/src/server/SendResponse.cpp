/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SendResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 09:39:10 by karim             #+#    #+#             */
/*   Updated: 2025/07/24 20:28:25 by karim            ###   ########.fr       */
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

void	ServerManager::transmitFileResponse(Client& client, int serverIndex) {

	if (client.getIsResponseSendable() == RESPONSE_PENDING)
		client.readTargetFileContent();

	if (client.getIsResponseSendable() == RESPONSE_READY) {
		size_t sentBytes;
		size_t bytesToSend = client.getBytesToReadFromTargetFile();

		std::string& response = client.getResponseHolder();
		
		try {
			if (!bytesToSend)
				bytesToSend = BYTES_TO_SEND;
			
			sentBytes = client.getSocket().send(response.c_str(), response.size());
			// std::cout << "sent Bytes: " << sentBytes << "\n";

			client._tempBuffer += response;	
			
			if (sentBytes > 0) {
				response.clear();
				std::string& buffer = client.getBufferedFileRemainder();
				if (!buffer.empty()) {	
					response = buffer;
					buffer.clear();
				}
				// printRequestAndResponse("New Response", response);
				client.analyzeResponseHolder();
			}
			else
				throwIfSocketError("send()");
			
		
		} catch (const std::runtime_error& e) {
			perror(e.what());
			_servers[serverIndex].closeConnection(client.getSocket().getFd());
		}
	}

	if (client.getResponseStats() == RESPONSE_SEND_DONE)
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
}

void    ServerManager::sendClientsResponse(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		
		if (it->second.getResponseInFlight()) {
			transmitResponse(it->second, serverIndex);
		}
		else if (it->second.getGetResponseInProgress() == GET_RESPONSE_ON)
			transmitFileResponse(it->second, serverIndex);
	}
	_servers[serverIndex].eraseMarked();
}