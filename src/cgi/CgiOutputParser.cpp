#include "CgiHandler.hpp"

void	CgiHandler::HandleDuplicates()
{
	if (key_holder == "Status" || key_holder == "Content-Type" || key_holder == "Content-Length")
		throw (BadCgiOutput("duplicate " + key_holder));
	else if (key_holder == "Set-Cookie"){
		if (ReachedMaxHeadersNumber())
			throw (BadCgiOutput("passed max headers number"));
		extra_cookie_values.push_back(value_holder);
	}
	else if (output_headers[key_holder].size() + value_holder.size() > HEADER_VALUE_SIZE_LIMIT)
		throw (BadCgiOutput("passed max header value size"));
	output_headers[key_holder] += ", " + value_holder;
}

void	CgiHandler::AddNewHeader()
{
	TrimSpaces(value_holder);
	if (key_holder.size() > HEADER_SIZE_LIMIT || value_holder.size() > HEADER_VALUE_SIZE_LIMIT)
		throw (BadCgiOutput("passed max header name/value size"));
	if (output_headers.find(key_holder) != output_headers.end())
		return (HandleDuplicates());
	if (ReachedMaxHeadersNumber())
		throw (BadCgiOutput("passed max headers number"));
	else if (key_holder == "Status")
		return (StatusValidator());
	else if (key_holder == "Content-Length")
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
	if (output_headers.find("Content-Type") == output_headers.end())
		throw (BadCgiOutput("Content-Type header not found"));
	else if (output_headers.find("Content-Length") != output_headers.end() 
			&& content_length < (signed)preserved_body.size())
		throw (BadCgiOutput("Content-Length value smaller than body size"));
}

void	CgiHandler::ParseOutputBuffer(const std::string &cgi_output_buff)
{
	bool	key_phase = (value_holder.empty()) ? true : false;

	for (unsigned int i = 0;i < cgi_output_buff.size();i++)
	{
		if (parsed_bytes_count >= HEADER_SIZE_LIMIT)
			throw (BadCgiOutput("passed header size limit"));
		SyntaxErrorsCheck(cgi_output_buff, i, key_phase);
		if (key_phase && cgi_output_buff[i] == ':'){
			key_phase = false;
			i++;
		}
		else if (Crlf_check(cgi_output_buff, i))
		{
			AddNewHeader();
			i += 2;
			key_phase = true;
			if (Crlf_check(cgi_output_buff, i)){//end of header
				preserved_body = cgi_output_buff.substr(i+2);
				PreBodyPhraseChecks();
				Body_phase = true;
				return ;
			}
		}
		if (key_phase)
			AppendCharToKey(key_holder, cgi_output_buff[i]);
		else
			AppendCharToValue(value_holder, cgi_output_buff[i]);
		parsed_bytes_count++;
	}
}
