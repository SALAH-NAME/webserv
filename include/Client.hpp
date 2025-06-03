/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/05/29 21:25:26 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstring>
#include <vector>
#include <sys/epoll.h>
#include "Request.hpp"

class Client {
	private:
		Request							request;
		int								socket_fd;
		int								serverSocket_fd;
		size_t							readBytes;
		std::string						requestHolder;
		std::string						responseHolder;
		int								outStatus;
		time_t								timeOut;

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

		// void		addNewEvent(struct epoll_event);
		// std::vector<struct epoll_event>&	getEvents(void);

		void	setOutStatus(int status);
		int	getOutStatus(void);
		void						clearRequestHolder(void);
		void		resetLastConnectionTime(void);
		time_t		getLastConnectionTime(void);
};

#endif