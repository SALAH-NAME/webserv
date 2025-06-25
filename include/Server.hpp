/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 18:39:03 by karim             #+#    #+#             */
/*   Updated: 2025/06/25 19:44:48 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerManager.hpp"

class Server {
	private:
		const ServerConfig&		_serverConfig;
		static int 				_id;
		int						_server_id;
		std::vector<int>		_sockets_fds;
		sockaddr_in				_Address;
		std::vector<int>		_ports;
		int						_domin;
		int						_type;
		int						_protocol;
		int						_nMaxBacklog; // this parameter is for listen()
		int 					_epfd; // epoll fd
		std::string				_2CRLF;
		bool					_isKeepAlive;
		bool					_isSocketOwner;
		char					_buffer[1024];
		size_t					_bufferSize;
		int						_timeOut;
		std::map<int, Client>	_clients;
		std::vector<int>		_clientsSockets;

		void					__init_attributes(void);
		void					setEventStatus(struct epoll_event&, int);


	public:
								Server(const ServerConfig&);
								~Server(void);

		int						get_id(void);
		void					setPort(std::vector<int>);
		void 					set__epfd(int );
		std::vector<int>&		get_sockets_fds();
		std::vector<int>&		get_clientsSockets(void);
		bool					verifyServer_sockets_fds(int);
		bool					verifyClientFD(int);
		void					incomingConnection(int);			
		void					checkTimeOut(void);
		bool					getIsSocketOwner(void);
		void					closeConnection(int);
		void    				receiveRequests(struct epoll_event&);
		void					sendResponses(struct epoll_event&);
		int						getTimeOut(void);
		std::map<int, Client>&	getClients(void);
};

std::vector<int>::iterator		get_iterator(int, std::vector<int>&);
void							printRequet(std::string);

#endif