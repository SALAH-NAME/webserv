/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SendResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 09:39:10 by karim             #+#    #+#             */
/*   Updated: 2025/07/08 14:26:56 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static std::string getResponseString(void) {
	std::string htmlContent =
			"<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"    <meta charset=\"UTF-8\">\n"
			"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
			"    <title>Welcome!</title>\n"
			"</head>\n"
			"<body style=\"font-family: sans-serif; background-color: #f4f4f4; margin: 40px; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); text-align: center;\">\n"
			"    <h1 style=\"color: #333; margin-bottom: 20px;\">Hello from My Server!</h1>\n"
			"    <p style=\"color: #666; line-height: 1.6; margin-bottom: 15px;\">You've reached a basic HTML page served by my simple web server.</p>\n"
			"    <p style=\"color: #007bff; font-weight: bold; margin-bottom: 15px;\">Enjoy the simplicity!</p>\n"
			"</body>\n"
			"</html>";
		
		std::stringstream ss;
		ss << htmlContent.length();
		std::string contentLength = ss.str();
		
		std::string response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + contentLength + "\r\n"
			"\r\n"
			+ htmlContent;

		return response;
}

static void	finalizeRequestHandling(Server& server, Client& client, int sentBytes) {
	client.setSentBytes(sentBytes);
	if (client.getSentBytes() == RESPONSESIZE){
		client.clearRequestHolder();
		client.resetSendBytes();
		client.setResponseInFlight(false);
		if (!client.getIsKeepAlive()) {
			server.closeConnection(client.getSocket().getFd());
		}
		// std::cout << "    ===> sent response <<=== \n";
	}
}

void	ServerManager::transmitResponse(Client& client, int serverIndex) {
	std::string response = getResponseString();
	int bytesToSendNow =  client.getBytesToSendNow();
	ssize_t sentBytes;

	try {
		sentBytes = client.getSocket().send(response.c_str() + client.getSentBytes(), bytesToSendNow);
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
		if (it->second.getResponseInFlight())
			transmitResponse(clients[it->first], serverIndex);
	}
	_servers[serverIndex].eraseMarked();
}