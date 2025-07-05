/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/07/05 22:03:21 by karim            ###   ########.fr       */
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
#define BYTES_TO_SEND 1000


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
		void				setEvent(int _epfd, struct epoll_event& event);
		void				setResponseInFlight(bool value);
		void				setSentBytes(size_t bytes);
		void				resetSendBytes(void);
		void				setIncomingDataDetected(int mode);

		void				clearRequestHolder(void);
		bool				parseRequest(void);
		void				prinfRequestinfos(void);
};

#endif