/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:39:03 by karim             #+#    #+#             */
/*   Updated: 2025/08/08 18:37:39 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerManager.hpp"

class Server {
	private:
		const ServerConfig&			_serverConfig;
		int 						_id;
		sockaddr_in					_Address;
		std::vector<unsigned int>	_ports;
		int							_domain;
		int							_type;
		int							_protocol;
		int							_nMaxBacklog;
		int 						_epfd;
		bool						_isKeepAlive;
		int							_timeOut;
		std::map<int, Client>		_clients;
		std::vector<int>			_markedForEraseClients;
		std::vector<Socket>			_listeningSockets;
		Socket						_transferSocket;
		
		void						initAttributes(int);
		void						setEventStatus(struct epoll_event&, int);
		void						setup_sockaddr(int port);
	public:
		/**/						Server(const ServerConfig&, size_t);
		/**/						~Server(void);

		int							getID(void);
		std::vector<Socket>&		getListeningSockets(void);
		std::map<int, Client>&		getClients(void);
		int							getTimeOut(void);
		std::vector<int>			getMarkedForEraseClients();
		Socket&						getTransferSocket(void);

		void						setPort(std::vector<int>);
		void 						setEPFD(int );

		bool						verifyServerSocketsFDs(int);
		std::map<int, Client>::iterator	verifyClientsFD(int);
		void						incomingConnection(int);			
		void						checkTimeOut(void);
		void						closeConnection(int);
		void						eraseMarked();
};

std::vector<int>::iterator			getIterator(int, std::vector<int>&);
void								printRequestAndResponse(std::string, std::string);

#endif