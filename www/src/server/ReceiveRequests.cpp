/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/07/24 20:29:40 by karim            ###   ########.fr       */
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
	(void)serverIndex, (void)clientSocket;

	
	std::memset(_buffer, 0, sizeof(_buffer));
	try {
		readbytes = client.getSocket().recv((void*)_buffer, BYTES_TO_READ);
		// std::cout << "read bytes ==> " << readbytes << " from : " << client.getSocket().getFd() << "\n";
		
		if (readbytes > 0) {
			client.resetLastConnectionTime();
			client.appendToHeaderPart(std::string(_buffer, readbytes)); // !! Append buffer to header-Part even if it contains Body-data  // READ THIS!!
			if (std::string(_buffer, readbytes) == "\r\n")
			{
				// in case of receive empty line (Press Enter) !!
				client.setIncomingDataDetectedFlag(INCOMING_DATA_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);	
			}
			// if ((headerEnd = client.getHeaderPart().find(_2CRLF)) != std::string::npos) {
			else if ((headerEnd = client.getHeaderPart().find(_2CRLF)) != std::string::npos) {
				// std::cout << "   ====>> request is completed <<=====\n";
				
				isolateAndRecordBody(client, headerEnd);
				
				// printRequestAndResponse("Header", client.getHeaderPart());
				// printRequestAndResponse("Body", client.getBodyPart());

				client.setIncomingDataDetectedFlag(INCOMING_DATA_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
			}
			
			HttpRequest &req = client.getHttpRequest();
			req.appendAndValidate(client.getHeaderPart());
			if (req.getState() == HttpRequest::STATE_ERROR)
			{
				client.setIncomingDataDetectedFlag(INCOMING_DATA_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
				return; // Return instead of throwing to allow response generation
			}
			
			// std::cout << "==== THIS INFO ====" << std::endl; // debug
			// req.printInfos(); // debug
			// std::cout << "==== THIS INFO ====" << std::endl; // debug
		
		}
		else
			throwIfSocketError("recv()");
	}
	catch (const HttpRequestException &e) {
		std::cout << "CATCH .....\n";
		std::string error_msg = "HTTP Request Error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingDataDetectedFlag(INCOMING_DATA_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
		// _servers[serverIndex].closeConnection(clientSocket); // 
	}
	catch (const std::runtime_error& e) {
		std::cout << "CATCH .....\n";
		std::string error_msg = "HTTP parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingDataDetectedFlag(INCOMING_DATA_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
		// perror(e.what());
		// _servers[serverIndex].closeConnection(clientSocket); //
	}
	catch (const std::exception &e) {
		std::cout << "CATCH .....\n";
		std::string error_msg = "Parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingDataDetectedFlag(INCOMING_DATA_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
		// _servers[serverIndex].closeConnection(clientSocket); //
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
		std::cout << "22222\n";
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
