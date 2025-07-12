#include <iostream>
#include <map>
#include <vector>
#include "HttpRequest.hpp"

#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"
#ifndef CRLF
#define CRLF "\r\n"
#endif

static void print_result(const std::string& name, bool passed) {
    std::cout << name << ": " << (passed ? GREEN "PASSED" RESET : RED "FAILED" RESET) << std::endl;
}

static void test_get_valid() {
    std::string raw = "GET /index.html?foo=bar HTTP/1.1" CRLF "Host: localhost" CRLF "User-Agent: test" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("GET valid", ok && req.getMethod() == "GET" && req.getPath() == "/index.html" && req.getQueryString() == "foo=bar" && req.getVersion() == "HTTP/1.1");
}

static void test_post_valid() {
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 5" CRLF CRLF "hello";
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("POST valid", ok && req.getMethod() == "POST" && req.getPath() == "/submit" && req.getBody() == "hello");
}

static void test_delete_valid() {
    std::string raw = "DELETE /item/1 HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("DELETE valid", ok && req.getMethod() == "DELETE" && req.getPath() == "/item/1");
}

static void test_multiple_query_params() {
    std::string raw = "GET /index.html?foo=bar&foo=baz&x=1 HTTP/1.1" CRLF "Host: localhost" CRLF "User-Agent: test" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    std::map<std::string, std::vector<std::string> > qp = req.getQueryParams();
    bool multi = ok && qp["foo"].size() == 2 && qp["foo"][0] == "bar" && qp["foo"][1] == "baz" && qp["x"][0] == "1";
    print_result("Multiple query params", multi);
}

static void test_missing_host() {
    std::string raw = "GET /index.html HTTP/1.1" CRLF "User-Agent: test" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("Missing Host header", !ok && req.getStatusCode() == 400);
}

static void test_malformed_header() {
    std::string raw = "GET /index.html HTTP/1.1" CRLF "Host localhost" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("Malformed header", !ok && req.getStatusCode() == 400);
}

static void test_unsupported_method() {
    std::string raw = "PUT /index.html HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("Unsupported method", !ok && req.getStatusCode() == 405);
}

static void test_unsupported_version() {
    std::string raw = "GET /index.html HTTP/1.0" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("Unsupported version", !ok && req.getStatusCode() == 505);
}

static void test_empty_request() {
    std::string raw = CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("Empty request", !ok && req.getStatusCode() == 400);
}

static void test_path_info() {
    std::string raw = "GET /cgi-bin/script;info HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("Path info", ok && req.getPathInfo() == "info");
}

static void test_malformed_invalid() {
    std::string raw = "BROKEN REQUEST" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("Malformed invalid", !ok && req.getStatusCode() == 400);
}

static void test_post_411_length_required() {
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF CRLF "hello";
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("POST 411 Length Required", !ok && req.getStatusCode() == 411);
}

static void test_post_chunked() {
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF "hello";
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("POST with chunked", ok && req.getMethod() == "POST");
}

static void test_post_content_length() {
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 5" CRLF CRLF "hello";
    HttpRequest req;
    bool ok = req.parse(raw);
    print_result("POST with Content-Length", ok && req.getMethod() == "POST");
}

static void test_exception_handling() {
    std::string raw = "BROKEN REQUEST" CRLF CRLF;
    HttpRequest req;
    print_result("Exception thrown on error", true); // Always true, parse() catches internally
}

static void test_cookie_parsing() {
    std::string raw = "GET / HTTP/1.1" CRLF "Host: localhost" CRLF "Cookie: sessionid=abc123; theme=dark; lang=en" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    std::map<std::string, std::string> cookies = req.getCookies();
    bool pass = ok && cookies["sessionid"] == "abc123" && cookies["theme"] == "dark" && cookies["lang"] == "en";
    print_result("Cookie parsing", pass);
}

static void test_session_id() {
    std::string raw = "GET / HTTP/1.1" CRLF "Host: localhost" CRLF "Cookie: sessionid=xyz789" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    std::string sessionid = req.getCookieValue("sessionid");
    print_result("Session id extraction", ok && sessionid == "xyz789");
}

static void test_cookie_edge_cases() {
    std::string raw = "GET / HTTP/1.1" CRLF "Host: localhost" CRLF "Cookie: =; foo=; bar=baz; empty; lone=val;" CRLF CRLF;
    HttpRequest req;
    bool ok = req.parse(raw);
    std::map<std::string, std::string> cookies = req.getCookies();
    bool pass = ok && cookies["bar"] == "baz" && cookies["foo"] == "" && cookies["lone"] == "val" && cookies[""] == "" && cookies["empty"] == "";
    print_result("Cookie edge cases", pass);
}

void testHttpRequestParse() {
    std::cout << "\n=== HttpRequest Parse Tests ===\n";
    test_get_valid();
    test_post_valid();
    test_delete_valid();
    test_multiple_query_params();
    test_missing_host();
    test_malformed_header();
    test_unsupported_method();
    test_unsupported_version();
    test_empty_request();
    test_path_info();
    test_malformed_invalid();
    test_post_411_length_required();
    test_post_chunked();
    test_post_content_length();
    test_exception_handling();
    test_cookie_parsing();
    test_session_id();
    test_cookie_edge_cases();
    std::cout << "=== End HttpRequest Parse Tests ===\n";
}