/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/06/30 12:38:54 by karim            ###   ########.fr       */
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

#define INCOMING_DATA_ON true
#define INCOMING_DATA_OFF false
#define RESPONSESIZE 746 // Fix size for the temp response
#define BYTES_TO_SEND 1


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
		bool				_incomingDataDetected;
		bool				_responseInFlight;
		int					_sentBytes;
		struct epoll_event	_event;
		bool				_isKeepAlive;
		int					_availableResponseBytes;
		int					_responseSize;

	public:
							Client(void);
							Client(int fd, int socket_fd);
							
		size_t				getReadBytes(void);
		int					getFD();
		std::string			getRequest(void);
		int					get_serverSocketFD(void);
		std::string&		getResponse(void);
		time_t				getLastConnectionTime(void);
		bool				getIncomingDataDetected(void);
		struct epoll_event&	getEvent();
		bool				getResponseInFlight(void);
		bool				getIsKeepAlive(void);
		int					getSentBytes(void);
		int					getBytesToSendNow(void);

		void				setReadBytes(size_t);
		void				appendToRequest(const std::string& requestData);
		void				set_serverSocketFD(int);
		void				setResponse(std::string );
		void				setIncomingDataFlag(bool flag);
		void				resetLastConnectionTime(void);
		void				setEvent(struct epoll_event& event);
		void				setEventStatus(int epfd, struct epoll_event& event);
		void				setResponseInFlight(bool value);
		void				setSentBytes(int bytes);
		void				resetSendBytes(void);
		// void				_availableResponseBytes();

		void				clearRequestHolder(void);
		bool				parseRequest(void);
};

#endif