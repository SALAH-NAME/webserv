#ifndef CONFIG_TOKENIZER_HPP
#define CONFIG_TOKENIZER_HPP

#include <cstddef>
#include <set>
#include <string>
#include <vector>

enum ConfigTokenType
{
	CTT_DIRECTIVE,
	CTT_SERVER,
	CTT_LOCATION,
	CTT_NUMBER,
	CTT_SIZE,
	CTT_STRING,
	CTT_SEMICOLON,
	CTT_OPEN_BRACE,
	CTT_CLOSE_BRACE,
	CTT_TILDE,
	CTT_CGI_REG,
	CTT_COMMENT,
	CTT_END_OF_FILE,
	CTT_ERROR
};

class ConfigTokenizer
{
public:
	struct TokenRecord
	{
		std::string			token;
		ConfigTokenType type;
		size_t					line;
		size_t					column;

		TokenRecord(const std::string& tok, ConfigTokenType typ, size_t l, size_t c)
				: token(tok), type(typ), line(l), column(c)
		{
		}
	};

	ConfigTokenizer();
	~ConfigTokenizer();

	bool							 empty() const;
	bool							 loadFromFile(const std::string& fileName);
	const TokenRecord& front() const;
	void							 pop();

	std::vector<TokenRecord> getAllTokens() const;
	size_t									 getCurrentPosition() const;
	void										 resetPosition();
	const TokenRecord&			 peek(size_t n = 1) const;
	bool										 isEnd() const;

	// Debugging
	void printTokens() const;

private:
	std::vector<TokenRecord> _tokens;
	size_t									 _currentPos;

	void tokenize(const std::string& content);
	void addToken(const std::string& token, ConfigTokenType type, size_t line,
								size_t column);

	bool isSpecialChar(char c) const;
	bool isDirectiveName(const std::string& token) const;
	bool isSizeSuffix(char c) const;
	bool isSize(const std::string& identifier) const;
	bool isNumber(const std::string& identifier) const;

	void processIdentifier(const std::string& content, size_t& pos, size_t& line,
												 size_t& col);
	void processComment(const std::string& content, size_t& pos, size_t& line,
											size_t& col);
	void processSymbol(const std::string& content, size_t& pos, size_t& line,
										 size_t& col);

	void skipWhitespace(const std::string& content, size_t& pos, size_t& line,
											size_t& col);

	std::set<std::string> _directiveNames;
	void									initializeDirectiveNames();
};

#endif // !CONFIG_TOKENIZER_HPP
