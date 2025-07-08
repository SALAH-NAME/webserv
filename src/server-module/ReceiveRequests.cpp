/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/07/08 12:45:49 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerManager.hpp"

void    ServerManager::collectRequestData(Client& client, int serverIndex) {
	int clientSocket = client.getSocket().getFd();
	ssize_t readbytes;

	memset(_buffer, 0, sizeof(_buffer));

	try {
		readbytes = client.getSocket().recv((void*)_buffer, BYTES_TO_READ);
		
		if (readbytes > 0) {
			client.appendToRequest(std::string(_buffer, readbytes));
			client.setReadBytes(readbytes);
			client.resetLastConnectionTime();
		
			if (client.getRequest().find(_2CRLF) != std::string::npos) {
				// std::cout << "   ====>> request is comleted <<=====\n";
				// printRequet(client.getRequest());
				if (client.parseRequest()) {
					// client.prinfRequestinfos();
					client.setResponseInFlight(true);
					client.setIncomingDataDetected(INCOMING_DATA_OFF);
				}
				else
					_servers[serverIndex].closeConnection(clientSocket);
			}
		}
		else
			throwIfSocketError("recv()");
			// _servers[serverIndex].closeConnection(clientSocket);
	} catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(clientSocket);
	}
}

void	ServerManager::receiveClientsData(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++){
		if (clients[it->first].getIncomingDataDetected() == INCOMING_DATA_ON)
			collectRequestData(clients[it->first], serverIndex);
	}
	_servers[serverIndex].eraseMarked();
}