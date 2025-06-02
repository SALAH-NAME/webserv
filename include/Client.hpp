/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/06/01 10:34:23 by karim            ###   ########.fr       */
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
		int								socket_fd;
		int								serverSocket_fd;
		size_t							readBytes;
		std::string						requestHolder;
		std::string						responseHolder;
		int								outStatus;
		time_t							timeOut;
		HttpRequest						requestInfos;
		const ServerConfig				*serverInfo;

	public:
		Client(void);
		Client(int fd, int socket_fd);

		void		setReadBytes(size_t bytes);
		size_t		getReadBytes(void);
		int			getFD();
		void		setRequest(std::string requestData);
		std::string	getRequest(void);
		void		set_serverSocketFD(int s_fd);
		int			get_serverSocketFD(void);

		void		setResponse(std::string response);
		std::string &		getResponse(void);

		void	setOutStatus(int status);
		int	getOutStatus(void);
		void						clearRequestHolder(void);
		void		resetLastConnectionTime(void);
		time_t		getLastConnectionTime(void);
		bool		parseRequest();

		void		routing(const std::vector<ServerConfig>& serversInfo);
};

#endif