#include "ConfigTokenizer.hpp"
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>

ConfigTokenizer::ConfigTokenizer() : _currentPos(0)
{
	initializeDirectiveNames();
}

ConfigTokenizer::~ConfigTokenizer() {}

void ConfigTokenizer::initializeDirectiveNames()
{
	static const char* directives[] = { "listen",
																			"host",
																			"server_name",
																			"error_page",
																			"client_max_body_size",
																			"root",
																			"index",
																			"allowed_methods",
																			"autoindex",
																			"return",
																			"upload_store",
																			"cgi_pass",
																			"cgi_timeout",
																			"session_enable",
																			"session_name",
																			"session_path",
																			"session_timeout",
																			"connection_timeout",
																			"error_log",
																			"access_log" };

	const size_t numDirectives = sizeof(directives) / sizeof(directives[0]);
	_directiveNames =
			std::set<std::string>(directives, directives + numDirectives);
}

bool ConfigTokenizer::loadFromFile(const std::string& fileName)
{
	std::ifstream file(fileName.c_str());
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << fileName << std::endl;
		return false;
	}

	std::stringstream _buffer;
	_buffer << file.rdbuf();
	std::string content = _buffer.str();
	file.close();

	_tokens.clear();
	_currentPos = 0;

	tokenize(content);
	return true;
}

bool ConfigTokenizer::empty() const
{
	return _tokens.empty() || _currentPos >= _tokens.size();
}

const ConfigTokenizer::TokenRecord& ConfigTokenizer::front() const
{
	if (_currentPos < _tokens.size())
		return _tokens[_currentPos];

	static TokenRecord dummyToken("", CTT_ERROR, 0, 0);
	return dummyToken;
}

void ConfigTokenizer::pop()
{
	if (_currentPos < _tokens.size())
		_currentPos++;
}

std::vector<ConfigTokenizer::TokenRecord> ConfigTokenizer::getAllTokens() const
{
	return _tokens;
}

size_t ConfigTokenizer::getCurrentPosition() const { return _currentPos; }

void ConfigTokenizer::resetPosition() { _currentPos = 0; }

const ConfigTokenizer::TokenRecord& ConfigTokenizer::peek(size_t n) const
{
	size_t peekPos = _currentPos + n - 1;
	if (peekPos < _tokens.size())
		return _tokens[peekPos];

	if (!_tokens.empty())
		return _tokens.back();

	static TokenRecord dummyToken("", CTT_ERROR, 0, 0);
	return dummyToken;
}

bool ConfigTokenizer::isEnd() const
{
	return _currentPos >= _tokens.size() ||
				 (_currentPos == _tokens.size() - 1 &&
					_tokens.back().type == CTT_END_OF_FILE);
}

void ConfigTokenizer::printTokens() const
{
	std::cout << "Tokens: \n";
	for (size_t i = 0; i < _tokens.size(); ++i)
	{
		const TokenRecord& record = _tokens[i];
		std::cout << "Token: '" << record.token << "', Type: " << record.type
							<< ", Line: " << record.line << ", Column: " << record.column;
		if (i == _currentPos)
			std::cout << " <-- Current";
		std::cout << std::endl;
	}
}

void ConfigTokenizer::tokenize(const std::string& content)
{
	size_t pos	= 0;
	size_t col	= 1;
	size_t line = 1;

	while (pos < content.length())
	{
		char currentChar = content[pos];

		if (isspace(currentChar))
		{
			skipWhitespace(content, pos, line, col);
			continue;
		}
		if (currentChar == '#')
		{
			processComment(content, pos, line, col);
			continue;
		}
		if (currentChar == '{' || currentChar == '}' || currentChar == ';' ||
				currentChar == '~')
		{
			processSymbol(content, pos, line, col);
			continue;
		}
		if (isalnum(currentChar) || isSpecialChar(currentChar))
		{
			processIdentifier(content, pos, line, col);
			continue;
		}

		std::string errorToken(1, currentChar);
		addToken(errorToken, CTT_ERROR, line, col);
		pos++;
		col++;
	}
	addToken("EOF", CTT_END_OF_FILE, line, col);
}

void ConfigTokenizer::addToken(const std::string& token, ConfigTokenType type,
															 size_t line, size_t column)
{
	_tokens.push_back(TokenRecord(token, type, line, column));
}

bool ConfigTokenizer::isSpecialChar(char c) const
{
	return c == '_' || c == '-' || c == '.' || c == '/';
}

bool ConfigTokenizer::isSizeSuffix(char c) const
{
	return c == 'K' || c == 'k' || c == 'M' || c == 'm' || c == 'G' || c == 'g';
}

bool ConfigTokenizer::isDirectiveName(const std::string& token) const
{
	return _directiveNames.find(token) != _directiveNames.end();
}

void ConfigTokenizer::skipWhitespace(const std::string& content, size_t& pos,
																		 size_t& line, size_t& col)
{
	while (pos < content.length() && isspace(content[pos]))
	{
		if (content[pos] == '\n')
		{
			line++;
			col = 1;
		}
		else
			col++;
		pos++;
	}
}

void ConfigTokenizer::processComment(const std::string& content, size_t& pos,
																		 size_t& line, size_t& col)
{
	std::string comment = "#";
	pos++;
	col++;

	while (pos < content.length() && content[pos] != '\n')
	{
		comment += content[pos];
		pos++;
		col++;
	}
	if (pos < content.length() && content[pos] == '\n')
	{
		pos++;
		line++;
		col = 1;
	}
}

void ConfigTokenizer::processSymbol(const std::string& content, size_t& pos,
																		size_t& line, size_t& col)
{
	size_t startCol = col;
	char	 symbol		= content[pos];

	ConfigTokenType type;
	switch (symbol)
	{
	case '{':
		type = CTT_OPEN_BRACE;
		break;
	case '}':
		type = CTT_CLOSE_BRACE;
		break;
	case ';':
		type = CTT_SEMICOLON;
		break;
	case '~':
		type = CTT_TILDE;
		break;
	default:
		type = CTT_ERROR;
		break;
	}

	std::string symbolStr(1, symbol);
	addToken(symbolStr, type, line, startCol);

	pos++;
	col++;
}

void ConfigTokenizer::processIdentifier(const std::string& content, size_t& pos,
																				size_t& line, size_t& col)
{
	size_t startCol = col;

	std::string identifier;
	while (pos < content.length() &&
				 (isalnum(content[pos]) || isSpecialChar(content[pos])))
	{
		identifier += content[pos];
		pos++;
		col++;
	}
	ConfigTokenType type;
	if (identifier == "server")
		type = CTT_SERVER;
	else if (identifier == "location")
		type = CTT_LOCATION;
	else if (isDirectiveName(identifier))
		type = CTT_DIRECTIVE;
	else if (isNumber(identifier))
		type = CTT_NUMBER;
	else if (isSize(identifier))
		type = CTT_SIZE;
	else
		type = CTT_STRING;
	addToken(identifier, type, line, startCol);
}

bool ConfigTokenizer::isSize(const std::string& identifier) const
{
	if (identifier.size() >= 2)
	{
		std::string::size_type pos = 0;
		while (pos < identifier.length() && isdigit(identifier[pos]))
			pos++;
		if (pos == identifier.length() - 1 && isSizeSuffix(identifier[pos]))
			return true;
	}
	return false;
}

bool ConfigTokenizer::isNumber(const std::string& identifier) const
{
	if (identifier.size() > 0)
	{
		std::string::size_type pos = 0;
		while (pos < identifier.length() && isdigit(identifier[pos]))
			pos++;
		if (pos >= identifier.length())
			return true;
	}
	return false;
}
