/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sendResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 09:39:10 by karim             #+#    #+#             */
/*   Updated: 2025/05/12 20:37:05 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

void    Server::sendResponses(void) {
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

	// std::cout << "response size: " << responseWaitQueue.size() << "\n";

	for (size_t i = 0; i < responseWaitQueue.size(); i++) {
		responses[responseWaitQueue[i]].setResponse(response);
	}

	
	for (size_t i = 0; i < responseWaitQueue.size(); i++) {
		// std::cout << "sending response to fd: " << responseWaitQueue[i] << "\n";
		response = responses[responseWaitQueue[i]].getResponse();
		ssize_t bytes_sent = send(responseWaitQueue[i],
			response.c_str(), response.length(), 0);

		if (bytes_sent < 0) {
			// ...
		}
		
		responses.erase(responseWaitQueue[i]);
		close(responseWaitQueue[i]);
		responseWaitQueue.erase(responseWaitQueue.begin() + i);
		i--;
	}
}
