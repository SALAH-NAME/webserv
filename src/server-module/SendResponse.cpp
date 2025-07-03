/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SendResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 09:39:10 by karim             #+#    #+#             */
/*   Updated: 2025/07/03 15:39:38 by karim            ###   ########.fr       */
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
			server.closeConnection(client.getFD());
			// std::cout << "   ====>> close connection with : " << client.getFD() << " <<=======\n";
		}
		// std::cout << "    ===> sent response <<=== \n";
	}
}

void    ServerManager::sendClientsResponse(Server& server) {

	std::string response = getResponseString();

	std::map<int, Client>& clients = server.getClients();
	std::vector<int>&	clientsSocket = server.getClientsSockets();
	ssize_t sentBytes;

	for (size_t i = 0; i < clientsSocket.size(); i++) {
		if (!clients[clientsSocket[i]].getResponseInFlight())
			continue ;
		
		int bytesToSendNow =  clients[clientsSocket[i]].getBytesToSendNow();
		sentBytes = send(clientsSocket[i], response.c_str() + clients[clientsSocket[i]].getSentBytes(),
							bytesToSendNow, 0);
		if (sentBytes == -1)
			server.closeConnection(clientsSocket[i]);
		else
			finalizeRequestHandling(server, clients[clientsSocket[i]], sentBytes);
	}
}
