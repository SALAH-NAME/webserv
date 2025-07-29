/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/07/29 15:38:02 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerManager.hpp"

void	isolateAndRecordBody(Client& client, size_t headerEnd) {
	std::string& headerPart = client.getHeaderPart();

	if (headerPart.size() == headerEnd + 4) {
		// no body-data received after header ==> no need to save
		client.setBodyDataPreloaded(BODY_DATA_PRELOADED_OFF);
		return ;
	}
	
	// some body-data received after header-data ==> it needs to be saved and removed from header part
	client.setRequestBodyPart(headerPart.substr(headerEnd + 4)); // here we save the body
	client.setIsRequestBodyWritable(WRITABLE);
	client.setHeaderPart(headerPart.substr(0, headerEnd + 4));
	// std::cout << "ISOLATED\n";
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
			client.appendToHeaderPart(std::string(_buffer, readbytes)); // !! Append buffer to header-Part even if it contains Body-data  // READ THIS!!0
			client.temp_header += std::string(_buffer, readbytes);
			// printRequestAndResponse("Header", client.getHeaderPart());
			if (std::string(_buffer, readbytes) == "\r\n")
			{
				// in case of receive empty line (Press Enter) !!
				client.setIncomingHeaderDataDetectedFlag(INCOMING_DATA_HEADER_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
			}
			else if ((headerEnd = client.getHeaderPart().find(_2CRLF)) != std::string::npos) {
			// if ((headerEnd = client.getHeaderPart().find(_2CRLF)) != std::string::npos) {
				// std::cout << "   ====>> request is completed <<=====\n";
				// printRequestAndResponse("Header", client.getHeaderPart());

				isolateAndRecordBody(client, headerEnd);
				// std::cout << "  ===>> is Preloaded : " << client.getBodyDataPreloaded() << "\n";
				// std::cout << "  ==>> isolated bytes: " << client.getBodyPart().size() << "\n";
				
				client.setIncomingHeaderDataDetectedFlag(INCOMING_DATA_HEADER_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
			}
			
			HttpRequest &req = client.getHttpRequest();
			req.appendAndValidate(client.getHeaderPart());
			if (req.getState() == HttpRequest::STATE_ERROR)
			{
				std::cout << "    ==>>> PARSING ERROR <<<====\n";exit(0);
				client.setIncomingHeaderDataDetectedFlag(INCOMING_DATA_HEADER_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
				return; // Return instead of throwing to allow response generation
			}
		}
		else
			throwIfSocketError("recv()");
	}
	catch (const HttpRequestException &e) {
		std::string error_msg = "HTTP Request Error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingHeaderDataDetectedFlag(INCOMING_DATA_HEADER_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
		_servers[serverIndex].closeConnection(clientSocket); // 
	}
	catch (const std::runtime_error& e) {
		std::string error_msg = "HTTP parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingHeaderDataDetectedFlag(INCOMING_DATA_HEADER_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
		// perror(e.what());
		_servers[serverIndex].closeConnection(clientSocket); //
	}
	catch (const std::exception &e) {
		std::string error_msg = "Parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingHeaderDataDetectedFlag(INCOMING_DATA_HEADER_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
		_servers[serverIndex].closeConnection(clientSocket);
	}
}

void	ServerManager::transferBodyToFile(Client& client, int serverIndex) {

	try {
		if (client.getIsRequestBodyWritable() == NOT_WRITABLE)
			client.receiveRequestBody();
		else if (client.getIsRequestBodyWritable() == WRITABLE)
			client.writeBodyToTargetFile();
	} catch (const std::runtime_error& e) {
		perror(e.what());
		_servers[serverIndex].closeConnection(client.getSocket().getFd());
	}
}

void	ServerManager::receiveClientsData(int serverIndex) {
	std::map<int, Client>& clients = _servers[serverIndex].getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (it->second.getIncomingHeaderDataDetectedFlag() == INCOMING_HEADER_DATA_ON) {
			// std::cout << " ***** incoming Header data from : " << it->second.getSocket().getFd() << "  ****\n";
			collectRequestData(it->second, serverIndex);
		}
		else if (it->second.getIncomingBodyDataDetectedFlag() == INCOMING_BODY_DATA_ON) {
			// std::cout << " ***** incoming Body data from : " << it->second.getSocket().getFd() << "  ****\n";
			transferBodyToFile(it->second, serverIndex);
		}
	}
	
	_servers[serverIndex].eraseMarked();
}
