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

bool Crlf_check(const std::string &str, unsigned int index)
{
	if (index + 1 > str.size() - 1)
		return false;
	return (str[index] == '\r' && str[index + 1] == '\n');
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
	if (code.size() != 3 || !isAllDigit(code) || std::atoi(code.c_str()) < 0 || std::atoi(code.c_str()) > 999
				|| !std::strchr("12345", code[0]))
		throw (BadCgiOutput("Invalid status code"));
	status_code = std::atoi(code.c_str());
	std::getline(status_stream, status_reason_phrase);
	key_holder.clear();
	value_holder.clear();
}
