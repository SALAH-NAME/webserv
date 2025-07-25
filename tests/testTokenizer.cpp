#include "ConfigTokenizer.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

bool createTestFile(const std::string &filename, const std::string &content)
{
	std::ofstream file(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Failed to create test file: " << filename << std::endl;
		return false;
	}

	file << content;
	file.close();
	return true;
}

std::string tokenTypeToString(ConfigTokenType type)
{
	switch (type)
	{
	case CTT_DIRECTIVE:
		return "DIRECTIVE";
	case CTT_SERVER:
		return "SERVER";
	case CTT_LOCATION:
		return "LOCATION";
	case CTT_NUMBER:
		return "NUMBER";
	case CTT_SIZE:
		return "SIZE";
	case CTT_STRING:
		return "STRING";
	case CTT_SEMICOLON:
		return "SEMICOLON";
	case CTT_OPEN_BRACE:
		return "OPEN_BRACE";
	case CTT_CLOSE_BRACE:
		return "CLOSE_BRACE";
	case CTT_TILDE:
		return "TILDE";
	case CTT_CGI_REG:
		return "CGI_REG";
	case CTT_COMMENT:
		return "COMMENT";
	case CTT_END_OF_FILE:
		return "END_OF_FILE";
	case CTT_ERROR:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}

void testTokenizer(const std::string &testName, const std::string &config)
{
	std::cout << "\n=== Testing: " << testName << " ===\n";

	std::string filename = "test_" + testName + ".conf";
	if (!createTestFile(filename, config))
	{
		std::cerr << "Test failed: Unable to create test file\n";
		return;
	}

	ConfigTokenizer tokenizer;
	if (!tokenizer.loadFromFile(filename))
	{
		std::cerr << "Test failed: Unable to load test file\n";
		return;
	}

	std::cout << "Tokens:\n";
	int tokenCount = 0;
	while (!tokenizer.empty())
	{
		const ConfigTokenizer::TokenRecord &token = tokenizer.front();
		std::cout << "  [" << tokenCount++ << "] "
							<< "Type: " << std::setw(12) << tokenTypeToString(token.type)
							<< " | Value: " << std::setw(20) << "'" << token.token << "'"
							<< " | Line: " << token.line << " | Col: " << token.column
							<< "\n";
		tokenizer.pop();
	}

	remove(filename.c_str());

	std::cout << "=== Test Complete ===\n";
}

void testBasicServerBlock()
{
	std::string test1 = "server {\n"
											"    listen 8080;\n"
											"    host 127.0.0.1;\n"
											"}\n";
	testTokenizer("basic_server", test1);
}

void testWhitespaceAndComments()
{
	std::string test2 =
			"# Global comment\n"
			"server { # Comment after brace\n"
			"    # Indented comment\n"
			"    listen 8080;host 127.0.0.1; # Comment after directives\n"
			"}\n";
	testTokenizer("comments_whitespace", test2);
}

void testComplexServerWithLocationBlocks()
{
	std::string test3 = "server {\n"
											"    listen 8080;\n"
											"    host 127.0.0.1;\n"
											"    server_name example.com www.example.com;\n"
											"    \n"
											"    location / {\n"
											"        root /var/www;\n"
											"        index index.html;\n"
											"        allowed_methods GET POST DELETE;\n"
											"    }\n"
											"    \n"
											"    location ~ .php {\n"
											"        root /var/www;\n"
											"        cgi_pass /usr/bin/php-cgi;\n"
											"    }\n"
											"}\n";
	testTokenizer("server_with_locations", test3);
}

void testSizeValues()
{
	std::string test4 = "client_max_body_size 10M;\n"
											"client_max_body_size 20m;\n"
											"client_max_body_size 1K;\n"
											"client_max_body_size 2k;\n"
											"client_max_body_size 3G;\n"
											"client_max_body_size 4g;\n";
	testTokenizer("size_values", test4);
}

void testErrorCase()
{
	std::string test5 = "server {\n"
											"    listen 8080@; # Invalid character\n"
											"    host 127.0.0.1;\n"
											"}\n";
	testTokenizer("error_cases", test5);
}

void testNestedBlocks()
{
	std::string test6 = "server {\n"
											"    listen 80;\n"
											"    server_name example.org;\n"
											"    \n"
											"    location / {\n"
											"        root /var/www/html;\n"
											"        \n"
											"    }\n"
											"    location /api {\n"
											"        proxy_pass http://backend;\n"
											"        client_max_body_size 5M;\n"
											"    }\n"
											"}\n";
	testTokenizer("nested_blocks", test6);
}

void testMultipleServerBlocks()
{
	std::string test7 = "server {\n"
											"    listen 80;\n"
											"    server_name example.com;\n"
											"    root /var/www/example.com;\n"
											"}\n"
											"\n"
											"server {\n"
											"    listen 443;\n"
											"    server_name secure.example.com;\n"
											"    ssl on;\n"
											"    root /var/www/secure;\n"
											"}\n";
	testTokenizer("multiple_servers", test7);
}
void testStringValuesWithSpacesAndSpecialCharacters()
{
	std::string test8 = "server {\n"
											"    server_name \"example domain.com\";\n"
											"    error_page 404 /custom_404.html;\n"
											"    add_header X-Custom \"Value with spaces\";\n"
											"}\n";
	testTokenizer("string_values", test8);
}

void testCgiRegistration()
{
	std::string test9 = "server {\n"
											"    location ~ \\.php$ {\n"
											"        cgi_pass /usr/bin/php-cgi;\n"
											"    }\n"
											"    location ~ \\.sh$ {\n"
											"        cgi_pass /usr/bin/perl;\n"
											"    }\n"
											"}\n";
	testTokenizer("cgi_registration", test9);
}
void testAllTokenizer()
{

	testBasicServerBlock();
	testWhitespaceAndComments();
	testComplexServerWithLocationBlocks();
	testSizeValues();
	testErrorCase();
	testNestedBlocks();
	testMultipleServerBlocks();
	testStringValuesWithSpacesAndSpecialCharacters();
	testCgiRegistration();
}

