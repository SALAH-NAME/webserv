/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:39:03 by karim             #+#    #+#             */
/*   Updated: 2025/07/05 22:15:34 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerManager.hpp"

class Server {
	private:
		const ServerConfig&		_serverConfig;
		static int 				_id;
		int						_serverID;
		std::vector<int>		_socketsFDs;
		sockaddr_in				_Address;
		std::vector<int>		_ports;
		int						_domin;
		int						_type;
		int						_protocol;
		int						_nMaxBacklog; // this parameter is for listen()
		int 					_epfd; // epoll fd
		bool					_isKeepAlive;
		bool					_isSocketOwner;
		char					_buffer[1024];
		size_t					_bufferSize;
		int						_timeOut;
		std::map<int, Client>	_clients;
		std::vector<int>		_markedForEraseClients;
		
		void					initAttributes(void);
		void					setEventStatus(struct epoll_event&, int);
	public:
								Server(const ServerConfig&);
								~Server(void);

		int						getID(void);
		std::vector<int>&		getSocketsFDs();
		bool					getIsSocketOwner(void);
		std::map<int, Client>&	getClients(void);
		int						getTimeOut(void);
		std::vector<int>		getMarkedForEraseClients();

		void					setPort(std::vector<int>);
		void 					setEPFD(int );

		bool					verifyServerSocketsFDs(int);
		bool					verifyClientsFD(int);
		void					incomingConnection(int);			
		void					checkTimeOut(void);
		void					closeConnection(int);
		void					eraseMarked();
};

std::vector<int>::iterator		getIterator(int, std::vector<int>&);
void							printRequet(std::string);

#endif