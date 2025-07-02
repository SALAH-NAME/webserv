/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/07/02 10:16:54 by karim            ###   ########.fr       */
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
#define RESPONSESIZE 746 // Fixed size for the temp response
#define BYTES_TO_SEND 100


class Client {
	private:
		int					_socketFD;
		int					_serverSocketFD;
		size_t				_readBytes;
		std::string			_requestHolder;
		std::string			_responseHolder;
		time_t				_timeOut;
		HttpRequest			_requestInfos;
		const ServerConfig*	_serverInfo;
		bool				_incomingDataDetected;
		bool				_responseInFlight;
		size_t				_sentBytes;
		struct epoll_event	_event;
		bool				_isKeepAlive;
		int					_availableResponseBytes;
		int					_responseSize;

	public:
							Client(void);
							Client(int fd, int socketFD);
							
		size_t				getReadBytes(void);
		int					getFD();
		std::string			getRequest(void);
		int					getServerSocketFD(void);
		std::string&		getResponse(void);
		time_t				getLastConnectionTime(void);
		bool				getIncomingDataDetected(void);
		struct epoll_event&	getEvent();
		bool				getResponseInFlight(void);
		bool				getIsKeepAlive(void);
		size_t				getSentBytes(void);
		int					getBytesToSendNow(void);

		void				setReadBytes(size_t);
		void				appendToRequest(const std::string& requestData);
		void				setServerSocketFD(int);
		void				setResponse(std::string );
		void				setIncomingDataFlag(bool flag);
		void				resetLastConnectionTime(void);
		void				setEvent(struct epoll_event& event);
		void				setEventStatus(int epfd);
		void				setResponseInFlight(bool value);
		void				setSentBytes(size_t bytes);
		void				resetSendBytes(void);

		void				clearRequestHolder(void);
		bool				parseRequest(void);
};

#endif