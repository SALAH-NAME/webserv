/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/07/29 15:37:14 by karim            ###   ########.fr       */
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

		// TO DO : 1.client addr || 2. port

		Socket				_socket;
		int					_serverSocketFD;
		std::string			_requestHeaderPart;
		std::string			_requestBodyPart;
		size_t				_responseSize;
		time_t				_lastTimeConnection;
		HttpRequest			_httpRequest;
		bool				_incomingHeaderDataDetected;
		bool				_responseHeaderFlag;
		bool				_responseBodyFlag;
		bool				_fullResponseFlag;
		size_t				_uploadedBytes;
		size_t				_sentBytes;
		bool				_isKeepAlive;
		bool				_generateInProcess;
		std::string			_responseHolder;
		ResponseHandler*	_responseHandler;
		bool				_incomingBodyDataDetectedFlag;
		size_t				_contentLength;
		bool				_isResponseBodySendable;
		bool				_isRequestBodyWritable;
		bool				_bodyDataPreloaded;

		void				isolateAndRecordExtraBytes(void);
		
		public:
		
		/**/				Client(Socket, int, const ServerConfig&);
		/**/				~Client();
		Client(const Client& other);
		
		std::string			_tempBuffer;
		int 				temp_size;
		std::string					temp_header;
		
		Socket&				getSocket();
		int					getServerSocketFD(void);
		time_t				getLastConnectionTime(void);
		bool				getIncomingHeaderDataDetectedFlag(void);
		
		std::string&		getRequestBodyPart(void);

		bool				getResponseHeaderFlag(void);
		bool				getResponseBodyFlag(void);
		bool				getFullResponseFlag(void);

		std::string&		getResponseHolder(void);
		
		bool				getIsKeepAlive(void);
		int					getBytesToSendNow(void);
		bool				getGenerateInProcess(void);
		HttpRequest&		getHttpRequest(void);
		std::string&		getHeaderPart(void);
		std::string&		getBodyPart(void);
		size_t				getResponseSize(void);
		bool				getIncomingBodyDataDetectedFlag(void);
		size_t				getUploadedBytes(void);
		bool				getBodyDataPreloaded(void);
		size_t				getContentLength(void);

		bool				getIsResponseBodySendable(void);

		size_t				getSavedBytes(void);
		
		bool				getIsRequestBodyWritable(void);

		void				appendToHeaderPart(const std::string& requestData);
		void				appendToBodyPart(const std::string& requestData);
		void				setServerSocketFD(int);
		void				resetLastConnectionTime(void);
		void				setEvent(int _epfd, struct epoll_event& event);
		void				setResponseHeaderFlag(bool value);
		void				setFullResponseFlag(bool value);
		void				setSentBytes(size_t bytes);
		void				resetSendBytes(void);
		void				setIncomingHeaderDataDetectedFlag(int mode);
		void				setGenerateResponseInProcess(bool);
		void				setResponseSize(size_t);
		void				setBodyDataPreloaded(bool);
		void				setIncomingBodyDataDetectedFlag(bool);
		void				setRequestBodyPart(std::string);
		void				resetUploadedBytes(void);

		void				setContentLength(int);
		void				resetContentLength(void);
		void				setHeaderPart(std::string);
		void				setUploadedBytes(size_t);
		
		void				setIsRequestBodyWritable(bool);

		bool				parseRequest(void);
		void				prinfRequestinfos(void);

		void				buildResponse();
		void				trimBufferedBodyToContentLength(void);
		void				readTargetFileContent(void);
		
		void				receiveRequestBody(void);

		bool				updateHeaderStateAfterSend(size_t);
		bool				sendFileBody(void);
		bool				readFileBody(void);
		void				writeBodyToTargetFile(void);
};

#endif