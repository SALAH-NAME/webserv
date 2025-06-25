/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/06/25 19:45:02 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstring>
#include <vector>
#include <sys/epoll.h>
#include "HttpRequest.hpp"
#include "ConfigManager.hpp"


class Client {
	private:
		int					_socket_fd;
		int					_serverSocket_fd;
		size_t				_readBytes;
		std::string			_requestHolder;
		std::string			_responseHolder;
		time_t				_timeOut;
		HttpRequest			_requestInfos;
		const ServerConfig*	_serverInfo;

	public:
							Client(void);
							Client(int fd, int socket_fd);
		void				setReadBytes(size_t);
		size_t				getReadBytes(void);
		int					getFD();
		void				setRequest(std::string);
		std::string			getRequest(void);
		void				set_serverSocketFD(int);
		int					get_serverSocketFD(void);
		void				setResponse(std::string );
		std::string&		getResponse(void);
		void				clearRequestHolder(void);
		void				resetLastConnectionTime(void);
		time_t				getLastConnectionTime(void);
		bool				parseRequest();

		// void				routing(const std::vector<ServerConfig>& serversInfo);
};

#endif