/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReceiveRequests.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 19:32:22 by karim             #+#    #+#             */
/*   Updated: 2025/07/19 18:29:47 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerManager.hpp"

void isolateAndRecordBody(Client &client, std::string tempBuffer, size_t headerEnd)
{
	if (tempBuffer.size() == headerEnd + 4)
	{
		// no body-data received after header ==> no need to save
		client.appendToHeaderPart(tempBuffer);
		client.setReadBytes(tempBuffer.size());
		return;
	}

	// some body-data received after header-data ==> it needs to be saved and removed from header part
	client.appendToHeaderPart(tempBuffer.substr(0, headerEnd + 4));
	client.setReadBytes(headerEnd + 4);
	client.appendToBodyPart(tempBuffer.substr(headerEnd + 4)); // here we save the body
}

// new version
void ServerManager::collectRequestData(Client &client, int serverIndex)
{
	int clientSocket = client.getSocket().getFd();
	ssize_t readbytes;
	size_t headerEnd;

	(void)serverIndex, (void)clientSocket;

	std::memset(_buffer, 0, sizeof(_buffer)); // use std

	try
	{
		readbytes = client.getSocket().recv((void *)_buffer, BYTES_TO_READ);
		// std::cout << "read bytes ==> " << readbytes << " from : " << client.getSocket().getFd() << "\n";

		if (readbytes > 0)
		{
			// printRequestAndResponse("Header", std::string(_buffer, readbytes));
			client.resetLastConnectionTime();

			// Append new data to the header part
			client.appendToHeaderPart(std::string(_buffer, readbytes));

			// Validate and parse incrementally after each receive
			HttpRequest &req = client.getHttpRequest();
			req.appendAndValidate(std::string(_buffer, readbytes));

			// Check if parsing is complete
			if (req.getState() == HttpRequest::STATE_BODY)
			{
				// Check if there's body data after headers
				if ((headerEnd = client.getHeaderPart().find(_2CRLF)) != std::string::npos)
					isolateAndRecordBody(client, client.getHeaderPart(), headerEnd);

				// printRequestAndResponse("Header", client.getHeaderPart());
				// printRequestAndResponse("Body", client.getBodyPart());

				client.setIncomingDataDetected(INCOMING_DATA_OFF);
				client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
				// client.prinfRequestinfos();
			}
			else if (req.getState() == HttpRequest::STATE_ERROR)
			{
				std::string error_msg = "HTTP parsing error: " + req.getErrorMsg();
				throw std::runtime_error(error_msg);
			}
			else
			{
				// Still waiting for more data
				client.setReadBytes(readbytes);
			}
		}
		else
		{
			throwIfSocketError("recv()");
		}
	}
	catch (const HttpRequestException &e)
	{
		std::string error_msg = "HTTP Request Error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		_servers[serverIndex].closeConnection(clientSocket);
	}
	catch (const std::runtime_error &e)
	{
		perror(e.what());
		_servers[serverIndex].closeConnection(clientSocket);
	}
	catch (const std::exception &e)
	{
		std::string error_msg = "Parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		_servers[serverIndex].closeConnection(clientSocket);
	}
}

void ServerManager::receiveClientsData(int serverIndex)
{
	std::map<int, Client> &clients = _servers[serverIndex].getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->second.getIncomingDataDetected() == INCOMING_DATA_ON)
			collectRequestData(it->second, serverIndex);
	}
	_servers[serverIndex].eraseMarked();
}
