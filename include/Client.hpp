/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/07/25 15:14:28 by karim            ###   ########.fr       */
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
		std::string			_responseHolder;
		ResponseHandler*	_responseHandler;
		bool				_shouldTransferBody;
		size_t				_bodySize;
		size_t				_contentLength;
		bool				_bodyDataPreloaded;

		bool				_isResponseSendable;
		size_t				_bytesToReadFromTargetFile;
		std::string			_bufferedFileRemainder;
		bool				_GetResponseInProgress;
		bool				_responseStats;

		
		void				isolateAndRecordExtraBytes(void);
		
		public:
		
		
		/**/				Client(Socket, int, const ServerConfig&);
		/**/				~Client();
		
		std::string			_tempBuffer;
		
		size_t				getReadBytes(void);
		Socket&				getSocket();
		int					getServerSocketFD(void);
		time_t				getLastConnectionTime(void);
		bool				getIncomingDataDetectedFlag(void);
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
		std::string&		getResponseHolder(void);
		bool				getShouldTransferBody(void);
		size_t				getBodySize(void);
		bool				getBodyDataPreloaded(void);
		size_t				getContentLength(void);
		
		bool				getGetResponseInProgress(void);
		bool				getIsResponseSendable(void);
		size_t				getBytesToReadFromTargetFile(void);
		std::string&		getBufferedFileRemainder(void);
		bool				getResponseStats(void);

		void				setReadBytes(size_t);
		void				appendToHeaderPart(const std::string& requestData);
		void				appendToBodyPart(const std::string& requestData);
		void				setServerSocketFD(int);
		void				resetLastConnectionTime(void);
		void				setEvent(int _epfd, struct epoll_event& event);
		void				setResponseInFlight(bool value);
		void				setSentBytes(size_t bytes);
		void				resetSendBytes(void);
		void				setIncomingDataDetectedFlag(int mode);
		void				setGenerateResponseInProcess(bool);
		void				setResponseSize(size_t);
		void				setAvailableResponseBytes(size_t);
		void				setBodyDataPreloaded(bool);
		void				setShouldTransferBody(bool);
		void				setRequestBodyPart(std::string);
		void				resetBodySize(void);
		void				setContentLength(int);
		void				resetContentLength(void);
		void				setHeaderPart(std::string);
	

		void				clearRequestHolder(void);
		bool				parseRequest(void);
		void				prinfRequestinfos(void);
		void				analyzeResponseHolder(void);

		void				buildResponse();
		void				trimBufferedBodyToContentLength(void);
		void				writeToTargetFile(const std::string& data);
		void				readTargetFileContent(void);
};

#endif