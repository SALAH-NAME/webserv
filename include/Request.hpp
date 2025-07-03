/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/05/12 16:58:36 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <cstring>
#include <vector>

class Request {
	private:
		int			socket_fd;
		int			serverSocket_fd;
		std::string	requestHolder;
	public:
		std::string	method;
		std::string	script;
		std::string	interpiter;
		std::string path_info;
		std::string query_string;
		std::string content_type;
		std::string content_length;
		std::string server_name;
		int			server_port;
		std::string client_addrs;
		int 		client_port;
		std::vector<std::pair<std::string, std::string> > headers;//exclude content-type and content-length

		Request(void);
		Request(int fd, int socket_fd);
		// Request(const Request& other);

		int			getFD();
		void		setRequest(std::string requestData);
		std::string	getRequest(void);
		void		set_serverSocketFD(int s_fd);
		int			get_serverSocketFD(void);
};

#endif