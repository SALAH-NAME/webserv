/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/07/19 18:28:28 by karim            ###   ########.fr       */
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
#include "ServerManager.hpp"

class Client {
	private:
		Socket				_socket;
		int					_serverSocketFD;
		size_t				_readBytes;
		std::string			_requestHeaderPart;
		std::string			_requestBodyPart;
		size_t				_responseSize;
		time_t				_lastTimeConnection;
		HttpRequest			_httpRequest;
		bool				_incomingDataDetected;
		bool				_responseInFlight;
		size_t				_sentBytes;
		bool				_isKeepAlive;
		size_t				_availableResponseBytes;
		bool				_generateInProcess;
		ResponseHandler*	_responseHandler;

	public:
		/**/				Client(Socket, int, const ServerConfig&);
		/**/				~Client();
		size_t				getReadBytes(void);
		Socket&				getSocket();
		int					getServerSocketFD(void);
		time_t				getLastConnectionTime(void);
		bool				getIncomingDataDetected(void);
		bool				getResponseInFlight(void);
		bool				getIsKeepAlive(void);
		size_t				getSentBytes(void);
		int					getBytesToSendNow(void);
		bool				getGenerateInProcess(void);
		HttpRequest&		getHttpRequest(void);
		std::string&		getHeaderPart(void);
		std::string&		getBodyPart(void);
		size_t				getResponseSize(void);
		size_t				getAvailableResponseBytes(void);

		void				setReadBytes(size_t);
		void				appendToHeaderPart(const std::string& requestData);
		void				appendToBodyPart(const std::string& requestData);
		void				setServerSocketFD(int);
		void				setIncomingDataFlag(bool flag);
		void				resetLastConnectionTime(void);
		void				setEvent(int _epfd, struct epoll_event& event);
		void				setResponseInFlight(bool value);
		void				setSentBytes(size_t bytes);
		void				resetSendBytes(void);
		void				setIncomingDataDetected(int mode);
		void				setGenerateResponseInProcess(bool);
		void				setResponseSize(size_t);
		void				setAvailableResponseBytes(size_t);

		void				clearRequestHolder(void);
		bool				parseRequest(void);
		void				prinfRequestinfos(void);

		void				buildResponse();
};

#endif