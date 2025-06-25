/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SendResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 09:39:10 by karim             #+#    #+#             */
/*   Updated: 2025/06/03 12:34:30 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void    Server::sendResponses(struct epoll_event& event) {

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

	int client_socket;
	ssize_t bytes_sent;
	
		client_socket = event.data.fd;

		if (!_clients[client_socket].getEventStatus())
			return ;

		bytes_sent = send(client_socket, response.c_str(), response.length(), 0);
		if (bytes_sent == -1) {
			// ...
		}

		if (bytes_sent < 0) {
			// ...
		}
		
		_clients[client_socket].setEventStatus(NOEVENT);
		event.events = EPOLLIN | EPOLLOUT;  // enable write temporarily
		epoll_ctl(_epfd, EPOLL_CTL_MOD, client_socket, &event);
		_clients[client_socket].clearRequestHolder();

		if (!_isKeepAlive)
			closeConnection(client_socket);
}
