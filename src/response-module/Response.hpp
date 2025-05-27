#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "ResponseManager.hpp"

class Response {
	private:

		// helpers
		bool		_isStatic;
		int			_flags;

		// infos
		std::string _host;
		std::string	_method;
		std::string	_path;
		std::string	_version;
		std::string _connection;

		// separators
		std::string _CRLF;
		std::string	_SP;
	
		// status line content
		std::string full_path;
		std::string	_statusCode;
		std::string	_statusMssg;
		std::string	_statusLine;
		
		// headers content
		std::string	Server;
		std::string	Date;
		std::string	ContentType;
		std::string	ContentLength;
		std::string	Location;
		std::string	Headers;

		// body
		std::string	Body;

		// holder
		std::string	responseHolder;

		void	constructStatusLine();
		void	checkMethodPart();
		int		ValidateMethod();
		void	checkpathPart();
		void	checkVersionPart();

		void	constructHeaders();
		void	setContentType(void);
		void	constructBody();



	public:
	
		Response(Request _parsedRequest);
		~Response(void);
	
		void		build();
		bool		isStatic(void);
		std::string	getResponse(void);
};

int		check_file(const std::string& path);
bool	containsUppercase(const std::string& str);
bool	startsWithHTTP(const std::string& str);
void    setDate(std::string& Date);

#endif