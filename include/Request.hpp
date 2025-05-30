/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/05/13 13:47:55 by karim            ###   ########.fr       */
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
		size_t		readBytes;
	public:
		std::string	method;
		std::string	path;
		std::string http_version;
		std::string	interpiter;//usually in the configuration
		std::string path_info;
		std::string query_string;
		std::string content_type;
		std::string content_length;
		std::string server_name;//config
		int			server_port;//config
		std::string client_addrs;//config
		int 		client_port;//config
		std::vector<std::pair<std::string, std::string> > headers;//exclude content-type and content-length

		Request(void);
		Request(int fd, int socket_fd);

		void		setReadBytes(size_t bytes);
		size_t		getReadBytes(void);
		int			getFD();
		void		setRequest(std::string requestData);
		std::string	getRequest(void);
		void		set_serverSocketFD(int s_fd);
		int			get_serverSocketFD(void);
};

#endif