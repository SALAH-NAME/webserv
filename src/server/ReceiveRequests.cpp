/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/07/23 13:23:49 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerManager.hpp"

void	isolateAndRecordBody(Client& client, size_t headerEnd) {
	std::string& headerPart = client.getHeaderPart();

	if (headerPart.size() == headerEnd + 4) {
		// no body-data received after header ==> no need to save
		client.setReadBytes(headerPart.size());
		return ;
	}
	
	// some body-data received after header-data ==> it needs to be saved and removed from header part
	client.setRequestBodyPart(headerPart.substr(headerEnd + 4)); // here we save the body
	client.setHeaderPart(headerPart.substr(0, headerEnd + 4));

	client.setBodyDataPreloaded(BODY_DATA_PRELOADED_ON);
}

void    ServerManager::collectRequestData(Client& client, int serverIndex) {
	int clientSocket = client.getSocket().getFd();
	ssize_t readbytes;
	size_t headerEnd;

	std::memset(_buffer, 0, sizeof(_buffer));
	try {
		readbytes = client.getSocket().recv((void*)_buffer, BYTES_TO_READ);
		// std::cout << "read bytes ==> " << readbytes << " from : " << client.getSocket().getFd() << "\n";
		
		if (readbytes > 0) {
			client.resetLastConnectionTime();
			client.appendToHeaderPart(std::string(_buffer, readbytes)); // !! Append buffer to header-Part even if it contains Body-data  // READ THIS!!
			
			if ((headerEnd = client.getHeaderPart().find(_2CRLF)) != std::string::npos) {
				// std::cout << "   ====>> request is completed <<=====\n";
				
				isolateAndRecordBody(client, headerEnd); // !! here we handle if Header-Part contains some of Body-data // READ THIS!!
				// !! Now we have Header-Part contains only Header-Data // READ THIS !!
				// !! And save the body-data in Body-Part if received // READ THIS !!
				
				// printRequestAndResponse("Header", client.getHeaderPart());
				// printRequestAndResponse("Body", client.getBodyPart());
				
				client.setIncomingDataDetectedFlag(INCOMING_DATA_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
			}
			// std::cout << "To validate {" << client.getLastReceivedHeaderData() << "}\n";
			// printRequestAndResponse("To validate", client.getLastReceivedHeaderData());
			
			HttpRequest &req = client.getHttpRequest();
			req.appendAndValidate(std::string(client.getLastReceivedHeaderData(), readbytes));
			// !! the string passed here is the last data received and appended to the Header-Part // READ THIS !!
			
			// client.prinfRequestinfos();
		}
		else
			throwIfSocketError("recv()");
	}
	catch (const HttpRequestException &e) {
		std::string error_msg = "HTTP Request Error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		_servers[serverIndex].closeConnection(clientSocket);
	}
	catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(clientSocket);
	}
	catch (const std::exception &e) {
		std::string error_msg = "Parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		_servers[serverIndex].closeConnection(clientSocket);
	}
}

void	ServerManager::transferBodyToFile(Client& client, int serverIndex) {
	if (client.getBodyDataPreloaded() == BODY_DATA_PRELOADED_ON) {
		client.trimBufferedBodyToContentLength();
		if (client.getShouldTransferBody() == TRANSFER_BODY_OFF)
			return ;
	}
	

	std::memset(_buffer, 0, sizeof(_buffer));
	try {
		size_t	readBytes = client.getSocket().recv(_buffer, BYTES_TO_READ);
		// std::cout << " ===> read bytes from Body ==> " << readBytes << "\n";
		if (readBytes > 0) {
			if ((client.getBodySize() + readBytes) >= client.getContentLength()) {
				readBytes = client.getContentLength() - client.getBodySize();
				client.setShouldTransferBody(TRANSFER_BODY_OFF);
				client.setResponseInFlight(true);
			}
			std::string bodyData(std::string(_buffer, readBytes));
			client.appendToBodyPart(bodyData);
			client.writeToTargetFile(bodyData);
		}
		else
			throwIfSocketError("recv()");
		
	} catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
	}
}

void	ServerManager::receiveClientsData(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++){
		if (it->second.getIncomingDataDetectedFlag() == INCOMING_HEADER_DATA_ON) {
			// std::cout << " ***** incoming data from : " << it->second.getSocket().getFd() << "  ****\n";
			collectRequestData(it->second, serverIndex);
		}
		else if (it->second.getShouldTransferBody() == TRANSFER_BODY_ON) {
			// std::cout << "   **** transfer body from: " << it->second.getSocket().getFd() << "  **** \n";	
			transferBodyToFile(it->second, serverIndex);
		}
	}
	_servers[serverIndex].eraseMarked();
}
