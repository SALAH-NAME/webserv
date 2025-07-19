/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/07/19 13:39:42 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerManager.hpp"

void	isolateAndRecordBody(Client& client, std::string tempBuffer, size_t headerEnd) {
	if (tempBuffer.size() == headerEnd + 4) {
		// no body-data received after header ==> no need to save
		client.appendToHeaderPart(tempBuffer);
		client.setReadBytes(tempBuffer.size());
		return ;
	}
	
	// some body-data received after header-data ==> it needs to be saved and removed from header part
	client.appendToHeaderPart(tempBuffer.substr(0, headerEnd + 4));
	client.setReadBytes(headerEnd + 4);
	client.appendToBodyPart(tempBuffer.substr(headerEnd + 4)); // here we save the body
	
}

void    ServerManager::collectRequestData(Client& client, int serverIndex) {
	int clientSocket = client.getSocket().getFd();
	ssize_t readbytes;
	size_t headerEnd;

	memset(_buffer, 0, sizeof(_buffer));

	try {
		readbytes = client.getSocket().recv((void*)_buffer, BYTES_TO_READ);
		// std::cout << "read bytes ==> " << readbytes << " from : " << client.getSocket().getFd() << "\n";
		
		if (readbytes > 0) {
			client.resetLastConnectionTime();
			if ((headerEnd = (std::string(_buffer, readbytes)).find(_2CRLF)) != std::string::npos) {
				// std::cout << "   ====>> request is completed <<=====\n";
				isolateAndRecordBody(client, std::string(_buffer, readbytes), headerEnd);
				// printRequestAndResponse("Header", client.getHeaderPart());
				// printRequestAndResponse("Body", client.getBodyPart());
				// exit(0);
				if (client.parseRequest()) {
					// client.prinfRequestinfos();
					// exit(0);
					client.setIncomingDataDetected(INCOMING_DATA_OFF);
					client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
				}
				else
					_servers[serverIndex].closeConnection(clientSocket);
			}
			else {
				client.appendToHeaderPart(std::string(_buffer, readbytes));
				client.setReadBytes(readbytes);
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
		if (it->second.getIncomingDataDetected() == INCOMING_DATA_ON)
			collectRequestData(it->second, serverIndex);
	}
	_servers[serverIndex].eraseMarked();
}
