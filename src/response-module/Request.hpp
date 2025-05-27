#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>

class Request {
	private:
		std::string _host;
		std::string	_method;
		std::string	_path;
		std::string	_version;
		std::string _connection;

		void	setHost(std::string);
		void	setMethod(std::string );
		void	setPath(std::string );
		void	setVersion(std::string );
		void	setConnection(std::string );
		
	public:
		Request(void);
		~Request(void);

		void		set_up(std::string host, std::string method, std::string path, std::string version, std::string connection);

		std::string	getHost(void);
		std::string	getMethod(void);
		std::string	getPath(void);
		std::string	getVersion(void);
		std::string	getConnection(void);

		void		printer(void);
};

#endif