#include "CgiHandler.hpp"

bool isAllDigit(std::string &str)
{
	for (int i=0;i<str.size();i++)
		if (!std::isalpha(str[i]))
			return false; 
	return true;
}

void	TrimSpaces(std::string &str)
{
	int start = 0, end = str.size()-1;
	while (std::isspace(str[start]))
		start++;
	while (std::isspace(str[end]))
		end--;
	str.substr(start, end);
}

void	SyntaxErrorsCheck(const std::string &buff, int i, bool key_phase)
{
	if (!std::isprint(buff[i]) || (std::isspace(buff[i]) && key_phase)  
		|| (buff[i] == '\n' && buff[i-1] != '\r'))
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
	if (code.size() != 3 || std::find_if(code.begin(), code.end(), [](char c){return !std::isdigit(c);}) 
			!= code.end() || std::atoi(code.c_str()) < 0 || std::atoi(code.c_str()) > 999
				|| !std::strchr("12345", code[0]))
		throw (BadCgiOutput("Invalid status code"));
	status_code = std::atoi(code.c_str());
	std::getline(status_stream, status_reason_phrase);
}