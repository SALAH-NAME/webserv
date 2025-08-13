#include "CgiHandler.hpp"

void	CgiHandler::HandleDuplicates()
{
	if (key_holder == "status" || key_holder == "content-type" || key_holder == "content-length")
		throw (BadCgiOutput("duplicate " + key_holder));
	else if (key_holder == "set-cookie"){
		if (ReachedMaxHeadersNumber())
			throw (BadCgiOutput("passed max headers number"));
		extra_cookie_values.push_back(value_holder);
		key_holder.clear();
		value_holder.clear();
		return ;
	}
	else if (output_headers[key_holder].size() + value_holder.size() > HEADER_VALUE_SIZE_LIMIT)
		throw (BadCgiOutput("passed max header value size"));
	output_headers[key_holder] += ", " + value_holder;
	key_holder.clear();
	value_holder.clear();
}

void ToLower(std::string &str)
{
	for (unsigned int i=0;i < str.size();i++)
		if (isupper(str[i]))
			str[i] = tolower(str[i]);
}

void	CgiHandler::AddNewHeader()
{
	TrimSpaces(value_holder);
	ToLower(key_holder);
	if (key_holder.size() > HEADER_SIZE_LIMIT || value_holder.size() > HEADER_VALUE_SIZE_LIMIT)
		throw (BadCgiOutput("passed max header name/value size"));
	if (output_headers.find(key_holder) != output_headers.end())
		return (HandleDuplicates());
	if (ReachedMaxHeadersNumber())
		throw (BadCgiOutput("passed max headers number"));
	else if (key_holder == "status")
		return (StatusValidator());
	else if (key_holder == "content-length")
		ContentLengthValidator();
	output_headers[key_holder] = value_holder;
	key_holder.clear();
	value_holder.clear();
}

void	AppendCharToKey(std::string &key, char c)
{
	static unsigned int count;

	if (count % 8 && key.size() >= HEADER_NAME_SIZE_LIMIT)
		throw (CgiHandler::BadCgiOutput("passed max header name size"));
	else
		key += c;
	count++;
}

void	AppendCharToValue(std::string &value, char c)
{
	static unsigned int count;

	if (count % 50 && value.size() >= HEADER_VALUE_SIZE_LIMIT)
		throw (CgiHandler::BadCgiOutput("passed max header name size"));
	else
		value += c;
	count++;
}

void	CgiHandler::PreBodyPhraseChecks()
{
	if (output_headers.find("content-type") == output_headers.end())
		throw (BadCgiOutput("Content-Type header not found"));
	else if (output_headers.find("content-length") != output_headers.end() 
			&& content_length < (signed)preserved_body.size())
		throw (BadCgiOutput("Content-Length value smaller than body size"));
	else if (prev_buf_l_char != '\n')
	{
			std::cout << "last char val: " << prev_buf_l_char << std::endl;
			throw (CgiHandler::BadCgiOutput("syntax error"));
	}
}

void	CgiHandler::SyntaxErrorsCheck(const std::string &buff, unsigned int i, bool key_phase)
{
	char prev_char = (i > 0) ? buff[i - 1] : prev_buf_l_char;
	char next_char = (i < buff.size() - 1) ? buff[i + 1] : 0;

//debugg block
	// if ((!std::isprint(buff[i]) && buff[i] != '\r'))
	// {
	// 		std::cout << "----first----\n";
	// 		std::cout << "prev-char: " << (int)prev_char << "\ncurrent char:" << (int)buff[i] 
	// 				<< "\nnext-char: " << (int)next_char << std::endl;
	// 		std::cout << "char index: " << i << std::endl;
	// 		throw (CgiHandler::BadCgiOutput("syntax error"));
	// }

	// if (std::isspace(buff[i]) && key_phase)
	// {
	// 		std::cout << "----second----\n";
	// 		std::cout << "prev-char: " << (int)prev_char << "\ncurrent char:" << (int)buff[i] 
	// 				<< "\nnext-char: " << (int)next_char << std::endl;
	// 		std::cout << "char index: " << i << std::endl;
	// 		throw (CgiHandler::BadCgiOutput("syntax error"));
	// }


	// if (buff[i] == '\n' && prev_char != '\r')
	// {
	// 		std::cout << "----third----\n";
	// 		std::cout << "prev-char: " << (int)prev_char << "\ncurrent char:" << (int)buff[i] 
	// 				<< "\nnext-char: " << (int)next_char << std::endl;
	// 		std::cout << "char index: " << i << std::endl;
	// 		throw (CgiHandler::BadCgiOutput("syntax error"));
	// }

	// if (buff[i] == '\r' && next_char != '\n')
	// {
	// 		std::cout << "----last----\n";
	// 		std::cout << "prev-char: " << (int)prev_char << "\ncurrent char:" << (int)buff[i] 
	// 				<< "\nnext-char: " << (int)next_char << std::endl;
	// 		std::cout << "char index: " << i << std::endl;
	// 		throw (CgiHandler::BadCgiOutput("syntax error"));
	// }

// //end of debugg block

	if ((!std::isprint(buff[i]) && buff[i] != '\r') || (std::isspace(buff[i]) && key_phase)  
		|| (buff[i] == '\n' && prev_char != '\r') || (buff[i] == '\r' && next_char != '\n'))
	{
			// std::cout << "prev-char: " << (int)prev_char << "\ncurrent char:" << (int)buff[i] 
			// 		<< "\nnext-char: " << (int)next_char << std::endl;
			// std::cout << "char index: " << i << std::endl;
			throw (CgiHandler::BadCgiOutput("syntax error"));
	}
}

void	CgiHandler::ParseOutputBuffer(const std::string &cgi_output_buff)
{
	bool	key_phase = (value_holder.empty()) ? true : false;

	// std::cout << "appending a buffer to the parser" << std::endl;
	unsigned int i = 0;

	std::cout << std::endl;
	for (i = 0; i < cgi_output_buff.size();i++)
	{
		if (parsed_bytes_count >= HEADER_SIZE_LIMIT)
			throw (BadCgiOutput("passed header size limit"));
		SyntaxErrorsCheck(cgi_output_buff, i, key_phase);
		if (key_phase && cgi_output_buff[i] == ':'){
			key_phase = false;
			parsed_bytes_count++;
			continue ;
		}
		if (Crlf_check(cgi_output_buff, i))
		{
			AddNewHeader();
			key_phase = true;
			if (Crlf_check(cgi_output_buff, i + 2)){//end of header
				prev_buf_l_char = '\n';
				parsed_bytes_count += 4;
				preserved_body = cgi_output_buff.substr(i + 4);
				PreBodyPhraseChecks();
				Body_phase = true;
				return ;
			}
			i += 1;
			parsed_bytes_count += 2;
			continue ;
		}
		if (key_phase)
			AppendCharToKey(key_holder, cgi_output_buff[i]);
		else
			AppendCharToValue(value_holder, cgi_output_buff[i]);
		parsed_bytes_count++;
	}
}
