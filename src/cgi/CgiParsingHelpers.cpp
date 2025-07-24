#include "CgiHandler.hpp"

bool isAllDigit(std::string &str)
{
	for (unsigned int i=0;i<str.size();i++)
		if (!std::isdigit(str[i]))
			return false; 
	return true;
}

void	TrimSpaces(std::string &str)
{
	if (str.empty())
		return;
	unsigned int start = 0, end = str.size() - 1;
	while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
		start++;
	while (end > start && std::isspace(static_cast<unsigned char>(str[end])))
		end--;
	str = str.substr(start, end - start + 1);
}

void	SyntaxErrorsCheck(const std::string &buff, unsigned int i, bool key_phase)
{
	char prev_char = (i > 0) ? buff[i - 1] : 0;
	char next_char = (i < buff.size() - 2) ? buff[i + 1] : 0;
	if ((!std::isprint(buff[i]) && buff[i] != '\r') || (std::isspace(buff[i]) && key_phase)  
		|| (buff[i] == '\n' && prev_char != '\r') || (buff[i] == '\r' && next_char != '\n'))
			throw (CgiHandler::BadCgiOutput("syntax error"));
}

bool Crlf_check(const std::string &str, unsigned int index)
{
	if (index+1 >= str.size())
		return false;
	return (str[index] == '\r' && str[index+1] == '\n');
}

bool CgiHandler::ReachedMaxHeadersNumber(){
	return (output_headers.size() + extra_cookie_values.size()>= HEADERS_NUMBER_LIMIT);
}

void CgiHandler::ContentLengthValidator()
{
	if (!isAllDigit(value_holder))
		throw (BadCgiOutput("non numeric Content-Length value"));
	std::stringstream ss(value_holder);
	ss >> content_length;
	if (ss.fail() || !ss.eof() || content_length < 0)
		throw (BadCgiOutput("Content-Length overflow or invalid value"));
}

void CgiHandler::StatusValidator()
{
	std::stringstream status_stream(value_holder);
	std::string code;
	status_stream >> code;
<<<<<<< HEAD
	if (code.size() != 3 || !isAllDigit(code) || std::atoi(code.c_str()) < 0 || std::atoi(code.c_str()) > 999
				|| !std::strchr("12345", code[0]))
=======
	if (code.size() != 3 || !isAllDigit(code) || std::atoi(code.c_str()) < 0
		|| std::atoi(code.c_str()) > 999 || !std::strchr("12345", code[0]))
>>>>>>> main
		throw (BadCgiOutput("Invalid status code"));
	status_code = std::atoi(code.c_str());
	std::getline(status_stream, status_reason_phrase);
}
