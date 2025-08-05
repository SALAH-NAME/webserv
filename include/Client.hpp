/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/06 12:38:44 by karim             #+#    #+#             */
/*   Updated: 2025/08/04 15:13:59 by karim            ###   ########.fr       */
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
		int					_epfd;
		int					_CGI_pipeFD;

		std::string			_requestHeaderPart;
		std::string			_requestBodyPart;
		std::string			_responseHolder;
		std::string			_pendingRequestDataHolder;
		
		time_t				_lastTimeConnection;
		size_t				_contentLength;
		size_t				_uploadedBytes;
		
		HttpRequest			_httpRequest;
		ResponseHandler*	_responseHandler;
		
		bool				_incomingHeaderDataDetected;
		bool				_incomingBodyDataDetectedFlag;
		bool				_responseHeaderFlag;
		bool				_responseBodyFlag;
		bool				_fullResponseFlag;
		bool				_isKeepAlive;
		bool				_generateInProcess;
		bool				_isResponseBodySendable;
		bool				_isRequestBodyWritable;
		bool				_bodyDataPreloadedFlag;
		bool				_requestDataPreloadedFlag;

		bool				_isCgiRequired;
		bool				_isPipeReadable;
		bool				_isPipeClosedByPeer;
		bool				_pipeReadComplete;

		bool				_setTargetFile;

		void				isolateAndRecordExtraBytes(void);
		void				generateDynamicResponse();
		void				generateStaticResponse();
		void				extractBodyFromPendingRequestHolder(void);
		
		public:

		/**/				Client(Socket, const ServerConfig&, int, ClientInfos);
		/**/				Client(const Client& other);
		/**/				~Client();
		
		std::string			_tempBuffer;
		int 				temp_size;
		std::string					temp_header;
		
		Socket&				getSocket();
		int					getCGI_pipeFD(void);
		time_t				getLastConnectionTime(void);
		bool				getIncomingHeaderDataDetectedFlag(void);
		
		std::string&		getRequestBodyPart(void);
		std::string&		getPendingRequestData(void);

		bool				getResponseHeaderFlag(void);
		bool				getResponseBodyFlag(void);
		bool				getFullResponseFlag(void);

		std::string&		getResponseHolder(void);
		ResponseHandler*	getResponseHandler(void);

		bool				getIsKeepAlive(void);
		int					getBytesToSendNow(void);
		bool				getGenerateInProcess(void);
		HttpRequest&		getHttpRequest(void);
		std::string&		getHeaderPart(void);
		std::string&		getBodyPart(void);
		size_t				getResponseSize(void);
		bool				getIncomingBodyDataDetectedFlag(void);
		size_t				getUploadedBytes(void);
		bool				getBodyDataPreloadedFlag(void);
		bool				setRequestDataPreloadedFlag(void);
		size_t				getContentLength(void);

		bool				getIsResponseBodySendable(void);

		size_t				getSavedBytes(void);
		
		bool				getIsRequestBodyWritable(void);
		bool				getIsCgiRequired(void);
		bool 				getIsPipeReadable(void);
		bool				getIsPipeClosedByPeer(void);

		bool				getSetTargetFile(void);

		void				appendToHeaderPart(const std::string& requestData);
		void				appendToBodyPart(const std::string& requestData);
		void				resetLastConnectionTime(void);
		void				setEvent(int _epfd, struct epoll_event& event);
		void				setResponseHeaderFlag(bool value);
		void				setFullResponseFlag(bool value);
		void				setResponseHolder(const std::string responseData);
		void				setSentBytes(size_t bytes);
		void				resetSendBytes(void);
		void				setIncomingHeaderDataDetectedFlag(int mode);
		void				setGenerateResponseInProcess(bool);
		void				setResponseSize(size_t);
		void				setBodyDataPreloadedFlag(bool);
		void				setRequestDataPreloadedFlag(bool value);
		void				setIncomingBodyDataDetectedFlag(bool);
		void				setRequestBodyPart(std::string);
		void				resetUploadedBytes(void);
		void				setPendingRequestData(std::string);

		void				setContentLength(int);
		void				resetContentLength(void);
		void				setHeaderPart(std::string);
		void				setUploadedBytes(size_t);
		
		void				setIsRequestBodyWritable(bool);
		void				setIsPipeReadable(bool);
		void				setIsPipeClosedByPeer(bool);
		void				setIsCgiRequired(bool);
		void				setPipeReadComplete(bool);

		void				setSetTargetFile(bool);

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
		void				closeAndDeregisterPipe(void);
};

#endif