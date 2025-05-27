#include "Response.hpp"

Response::Response(Request _parsedRequest) {
	_CRLF = "\r\n";
	_SP = " ";
	_host = _parsedRequest.getHost();
	_method = _parsedRequest.getMethod();
	_path = _parsedRequest.getPath();
	_version = _parsedRequest.getVersion();
	_connection = "Connection: " + _parsedRequest.getConnection();
	_flags = 0;
	_isStatic = true;
	Server = "Server: webserv";
	ContentLength = "Content-Length: ";
	ContentType = "Content-Type: ";
	Date = "Date: ";
	Location = "Location: ";
}

Response::~Response(void) {}

bool		Response::isStatic(void) {
	return _isStatic;
}

///// ------------- Status Line -----------

int	Response::ValidateMethod() {
	if (_method.empty())
		return NOTEXIST;
	else if (_method == "GET" || _method == "POST" || _method == "DELETE")
		return VALID;
	else if (!containsUppercase(_method))
		return NOTUPPER;
	else
		return NOTALLOWED;
}

void	Response::checkMethodPart() {
	int status = ValidateMethod();
	if (status == NOTEXIST || status == NOTUPPER) {
		_statusCode = "400";
		_statusMssg = "Bad Request";
	}
	else if (status == NOTALLOWED) {
		_statusCode = "405";
		_statusMssg = "Not Allowed";
		full_path = "/ErrorPages/Forbidden.html";
	}
	else
		_flags |= VALID_METHOD;
}

// ------ Path Part -----------------------

void	Response::checkpathPart() {
	std::string root = "/var/www/html";

	if (_path[0] == '/') {
		if (_path.find("..") != std::string::npos) {
			full_path = "ErrorPages/Bad-Request.html";
			_statusCode = "400";
			_statusMssg = "Bad Request";
		}
		else {
			full_path = root + _path;
			
			if (_path == "/") {
				_path = "/index.html";
				full_path = "webServPages" + _path;
			}
				
			int statusCode = check_file(full_path);

			if (statusCode == 404) {
				full_path = "ErrorPages/Non-Found.html";
				_statusCode = "404";
				_statusMssg = "Not Found";
			}
			else if (statusCode == 403) {
				full_path = "ErrorPages/Forbidden.html";
				_statusCode = "403";
				_statusMssg = "Forbidden";
			}
			else if (statusCode == 301) {
				full_path = "ErrorPages/Moved-Permanently.html";
				_statusCode = "301";
				_statusMssg = "Moved Permanently";
			}
			else 
				_flags |= VALID_PATH;
		}
	}
	else {
		full_path = "ErrorPages/Bad-Request.html";
		_statusCode = "400";
		_statusMssg = "Bad Request";
	}
}

// -------- Version Part -------------
void	Response::checkVersionPart()
{
	if (_version == "HTTP/1.1") {
		_flags |= VALID_VERSION;
		_statusCode = "200";
		_statusMssg = "OK";
	}
	else {
		if (startsWithHTTP(_version)) {
			_statusCode = "505";
			_statusMssg = "HTTP Version Not Supported";
			full_path = "ErrorPages/Not-Supported.html";
		}
		else {
			_statusCode =  "400";
			_statusMssg =  "Bad Request";
			full_path =  "ErrorPages/Bad-Request.html";
		}
	}
}


void Response::constructStatusLine() {
	checkMethodPart();
	if (_flags == VALID_METHOD) {
		checkpathPart();
		if (_flags == VALID_METHOD_AND_PATH) {
			checkVersionPart();
		}
	}
	_statusLine = _version + _SP + _statusCode + _SP + _statusMssg + _CRLF;
}

// ----------------------------------------------------

void	Response::setContentType(void) {
	int poss;

	if (_flags != STATUSLINE_VALID)
		ContentType += "text/html";
	else {
		poss = _path.rfind('.');
		std::string extension = _path.substr(poss + 1);

		if (extension == "html" || extension == "htm")
			ContentType += "text/html";
		else if (extension == "css")
			ContentType += "text/css";
		else if (extension == "js")
			ContentType += "application/javascript";
		else if (extension == "json")
			ContentType += "application/json";
		else if (extension == "png")
			ContentType += "image/png";
		else if (extension == "jpg" || extension == "jpeg")
			ContentType += "image/jpeg";
		else if (extension == "gif")
			ContentType += "image/gif";
		else if (extension == "svg")
			ContentType += "image/svg+xml";
		else if (extension == "txt")
			ContentType += "text/plain";
		else if (extension == "pdf")
			ContentType += "application/pdf";
		else
			ContentType += "application/octet-stream"; // default/fallback
	}
}

void	Response::constructHeaders() {
	std::ostringstream oss;

	setDate(Date);
	setContentType();
	oss << Body.size();
	ContentLength += oss.str();

	Headers = 	Server + _CRLF +
				Date + _CRLF +
				ContentType + _CRLF +
				ContentLength + _CRLF;
	if (_statusCode == "301") {
		Location += "http://" + _host + _path + "/";
		Headers += Location + _CRLF;
	}

	Headers += _connection + _CRLF;
}

void Response::constructBody() {
	std::string buffer;
	std::ifstream file(full_path.c_str());

	while (getline(file, buffer)) {
		Body += buffer;
		if (!file.eof())
            Body += "\n";
	}
}

void Response::build() {
	constructStatusLine();
	constructBody();
	constructHeaders();
	responseHolder = _statusLine + Headers + _CRLF + Body;
}

std::string	Response::getResponse(void) {
	return responseHolder;
}
