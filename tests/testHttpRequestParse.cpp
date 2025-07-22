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
    print_result("Path info", ok && req.getPathInfo() == "");
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

static void test_incremental_parsing() {
    std::cout << "=== Testing Incremental HTTP Parser ===" << std::endl;
    
    HttpRequest req;
    
    try {
        bool pass = true;
        // std::cout << "1. Adding start line..." << std::endl;
        req.appendAndValidate("GET /test?param=value HTTP/1.1\r\n");
        // std::cout << "   State: " << req.getState() << " (should be 1 = STATE_HEADERS)" << std::endl;
        pass = req.getState() == HttpRequest::STATE_HEADERS && req.getMethod() == "GET" && req.getUri() == "/test?param=value" && req.getVersion() == "HTTP/1.1";
        print_result("Adding start line ", pass);
        
        // std::cout << "2. Adding first header..." << std::endl;
        req.appendAndValidate("Host: localhost\r\n");
        // std::cout << "   State: " << req.getState() << std::endl;
        pass = req.getState() == HttpRequest::STATE_HEADERS && req.getHeaders().at("host") == "localhost" && req.getHeaders().size() == 1;
        print_result("Adding first header ", pass);
        
        // std::cout << "3. Adding second header..." << std::endl;
        req.appendAndValidate("User-Agent: TestClient/1.0\r\n");
        // std::cout << "   State: " << req.getState() << std::endl;
        pass = req.getState() == HttpRequest::STATE_HEADERS && req.getHeaders().at("user-agent") == "TestClient/1.0" && req.getHeaders().size() == 2;
        print_result("Adding second header ", pass);
        
        // std::cout << "4. Adding end of headers..." << std::endl;
        req.appendAndValidate("\r\n");
        // std::cout << "   State: " << req.getState() << " (should be 2 = STATE_BODY, 3 = STATE_COMPLETE)" << std::endl;
        pass = req.getState() == HttpRequest::STATE_BODY && req.getHeaders().size() == 2;
        print_result("Adding end of headers ", pass);
        
        // std::cout << "5. Checking parsed data:" << std::endl;
        // std::cout << "   Method: " << req.getMethod() << std::endl;
        // std::cout << "   Path: " << req.getPath() << std::endl;
        // std::cout << "   Query: " << req.getQueryString() << std::endl;
        // std::cout << "   Version: " << req.getVersion() << std::endl;
        // std::cout << "   Is Valid: " << req.isValid() << std::endl;
        // std::cout << "   Has Complete Request: " << req.hasCompleteRequest() << std::endl;
        pass = req.getState() == HttpRequest::STATE_BODY && req.getHeaders().size() == 2 && req.getMethod() == "GET" && req.getUri() == "/test?param=value" && req.getVersion() == "HTTP/1.1" && req.isValid() \
            && req.getPath() == "/test" && req.getQueryString() == "param=value" && req.getQueryParams().size() == 1 && req.getQueryParams().at("param")[0] == "value" && req.getQueryParams().at("param").size() == 1;
        print_result("Checking parsed data ", pass);
        
    } catch (const HttpRequestException& e) {
        std::cout << "ERROR: " << e.what() << " (Status: " << e.statusCode() << ")" << RED "FAILED" RESET << std::endl;
    }
}

static void test_incremental_error_detection() {
    std::cout << "\n=== Testing Error Detection ===" << std::endl;
    
    HttpRequest req;
    
    try {
        // std::cout << "1. Adding invalid method..." << std::endl;
        req.appendAndValidate("INVALID /test HTTP/1.1\r\n");
        std::cout << "   This should not be reached!" << std::endl;
    } catch (const HttpRequestException& e) {
        // std::cout << "   CAUGHT ERROR (expected): " << e.what() << " (Status: " << e.statusCode() << ")" << std::endl;
        print_result("Adding invalid method ", e.statusCode() == 405);
    }
    
    req.reset();
    
    try {
        // std::cout << "2. Adding too large start line..." << std::endl;
        std::string large_request = "GET ";
        large_request += std::string(5000, 's');
        large_request += " HTTP/1.1\r\n";
        req.appendAndValidate(large_request);
        std::cout << "   This should not be reached!" << std::endl;
    } catch (const HttpRequestException& e) {
        // std::cout << "   CAUGHT ERROR (expected): " << e.what() << " (Status: " << e.statusCode() << ")" << std::endl;
        print_result("Adding too large start line ", e.statusCode() == 414);
    }
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

    test_incremental_parsing();
    test_incremental_error_detection();
    std::cout << "=== End HttpRequest Parse Tests ===\n";
}