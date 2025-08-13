
#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerManager.hpp"

class Server {
	private:
		const std::vector<ServerConfig>&	_allServersConfig;
		const ServerConfig&					_serverConfig;
		int 						_id;
		sockaddr_in					_Address;
		std::vector<unsigned int>	_ports;
		int							_domain;
		int							_type;
		int							_protocol;
		int							_nMaxBacklog;
		int 						_epfd;
		int							_timeOut;
		std::map<int, Client>		_clients;
		std::vector<int>			_markedForEraseClients;
		std::vector<Socket>			_listeningSockets;
		std::vector<int>			_markedForEraseUnusedClient;
		Socket						_transferSocket;
		
		void						initAttributes(int);
		void						setEventStatus(struct epoll_event&, int);
		void						setup_sockaddr(int port);
	public:
		/**/						Server(const std::vector<ServerConfig>&, const ServerConfig&, size_t);
		/**/						~Server(void);

		const ServerConfig&			getConfig(void);
		int							getID(void);
		const std::vector<unsigned int>	getPorts(void);
		std::vector<Socket>&		getListeningSockets(void);
		std::vector<int>&			getMarkedForEraseUnusedSocket(void);
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