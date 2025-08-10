#include <iostream>
#include <map>
#include <vector>
#include "HttpRequest.hpp"

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"
#ifndef CRLF
#define CRLF "\r\n"
#endif

static void print_result(const std::string &name, bool passed)
{
    std::cout << name << ": " << (passed ? GREEN "PASSED" RESET : RED "FAILED" RESET) << std::endl;
}

static void test_get_valid()
{
    std::string raw = "GET /index.html?foo=bar HTTP/1.1" CRLF "Host: localhost" CRLF "User-Agent: test" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    print_result("GET valid", ok && req.getMethod() == "GET" && req.getPath() == "/index.html" && req.getQueryString() == "foo=bar" && req.getVersion() == "HTTP/1.1");
}

static void test_post_valid()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 5" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    print_result("POST valid", ok && req.getMethod() == "POST" && req.getPath() == "/submit");
}

static void test_post_valid_no_body()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 0" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    print_result("POST valid without Body", ok && req.getMethod() == "POST" && req.getPath() == "/submit" && req.getBody().empty());
}

static void test_delete_valid()
{
    std::string raw = "DELETE /item/1 HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    print_result("DELETE valid", ok && req.getMethod() == "DELETE" && req.getPath() == "/item/1");
}

static void test_multiple_query_params()
{
    std::string raw = "GET /index.html?foo=bar&foo=baz&x=1 HTTP/1.1" CRLF "Host: localhost" CRLF "User-Agent: test" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    std::map<std::string, std::vector<std::string> > qp = req.getQueryParams();
    bool multi = ok && qp["foo"].size() == 2 && qp["foo"][0] == "bar" && qp["foo"][1] == "baz" && qp["x"][0] == "1";
    print_result("Multiple query params", multi);
}

static void test_missing_host()
{
    std::string raw = "GET /index.html HTTP/1.1" CRLF "User-Agent: test" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        bool ok = req.isValid();
        print_result("Missing Host header", !ok && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Missing Host header", ex.statusCode() == 400);
    }
}

static void test_malformed_header()
{
    std::string raw = "GET /index.html HTTP/1.1" CRLF "Host localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        bool ok = req.isValid();
        print_result("Malformed header", !ok && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Malformed header", ex.statusCode() == 400);
    }
}

static void test_unsupported_method()
{
    std::string raw = "PUT /index.html HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        bool ok = req.isValid();
        print_result("Unsupported method", !ok && req.getStatusCode() == 405);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Unsupported method", ex.statusCode() == 405);
    }
}

static void test_unsupported_version()
{
    std::string raw = "GET /index.html HTTP/2.0" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        bool ok = req.isValid();
        print_result("Unsupported version", !ok && req.getStatusCode() == 505);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Unsupported version", ex.statusCode() == 505);
    }
}

static void test_empty_request()
{
    std::string raw = CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        bool ok = req.isValid();
        print_result("Empty request", !ok && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Empty request", ex.statusCode() == 400);
    }
}

static void test_path_info()
{
    std::string raw = "GET /cgi-bin/script;info HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    print_result("Path info", ok && req.getPathInfo() == "");
}

static void test_malformed_invalid()
{
    std::string raw = "BROKEN REQUEST" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        bool ok = req.isValid();
        print_result("Malformed invalid", !ok && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Malformed invalid", ex.statusCode() == 400);
    }
}

static void test_post_411_length_required()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        bool ok = req.isValid();
        print_result("POST 411 Length Required", !ok && req.getStatusCode() == 411);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("POST 411 Length Required", ex.statusCode() == 411);
    }
}

static void test_post_chunked()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    print_result("POST with chunked", ok && req.getMethod() == "POST");
}

static void test_post_content_length()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 5" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    print_result("POST with Content-Length", ok && req.getMethod() == "POST");
}

static void test_cookie_parsing()
{
    std::string raw = "GET / HTTP/1.1" CRLF "Host: localhost" CRLF "Cookie: sessionid=abc123; theme=dark; lang=en" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    std::map<std::string, std::string> cookies = req.getCookies();
    bool pass = ok && cookies["sessionid"] == "abc123" && cookies["theme"] == "dark" && cookies["lang"] == "en";
    print_result("Cookie parsing", pass);
}

static void test_session_id()
{
    std::string raw = "GET / HTTP/1.1" CRLF "Host: localhost" CRLF "Cookie: sessionid=xyz789" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    std::string sessionid = req.getCookieValue("sessionid");
    print_result("Session id extraction", ok && sessionid == "xyz789");
}

static void test_cookie_edge_cases()
{
    std::string raw = "GET / HTTP/1.1" CRLF "Host: localhost" CRLF "Cookie: =; foo=; bar=baz; empty; lone=val;" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    req.appendAndValidate(buffer);
    bool ok = req.isValid();
    std::map<std::string, std::string> cookies = req.getCookies();
    bool pass = ok && cookies["bar"] == "baz" && cookies["foo"] == "" && cookies["lone"] == "val" && cookies[""] == "" && cookies["empty"] == "";
    print_result("Cookie edge cases", pass);
}

static void test_incremental_parsing()
{
    std::cout << "=== Testing Incremental HTTP Parser ===" << std::endl;

    HttpRequest req;

    try
    {
        bool pass = true;
        std::string line1 = "GET /test?param=value HTTP/1.1\r\n";
        req.appendAndValidate(line1);
        pass = req.getState() == HttpRequest::STATE_HEADERS && req.getMethod() == "GET" && req.getUri() == "/test?param=value" && req.getVersion() == "HTTP/1.1";
        print_result("Adding start line ", pass);

        std::string line2 = "Host: localhost\r\n";
        req.appendAndValidate(line2);
        pass = req.getState() == HttpRequest::STATE_HEADERS && req.getHeaders().at("host") == "localhost" && req.getHeaders().size() == 1;
        print_result("Adding first header ", pass);

        std::string line3 = "User-Agent: TestClient/1.0\r\n";
        req.appendAndValidate(line3);
        pass = req.getState() == HttpRequest::STATE_HEADERS && req.getHeaders().at("user-agent") == "TestClient/1.0" && req.getHeaders().size() == 2;
        print_result("Adding second header ", pass);

        std::string line4 = "\r\n";
        req.appendAndValidate(line4);
        pass = req.getState() == HttpRequest::STATE_BODY && req.getHeaders().size() == 2;
        print_result("Adding end of headers ", pass);

        pass = req.getState() == HttpRequest::STATE_BODY && req.getHeaders().size() == 2 && req.getMethod() == "GET" && req.getUri() == "/test?param=value" && req.getVersion() == "HTTP/1.1" && req.isValid() && req.getPath() == "/test" && req.getQueryString() == "param=value" && req.getQueryParams().size() == 1 && req.getQueryParams().at("param")[0] == "value" && req.getQueryParams().at("param").size() == 1;
        print_result("Checking parsed data ", pass);
    }
    catch (const HttpRequestException &e)
    {
        std::cout << "ERROR: " << e.what() << " (Status: " << e.statusCode() << ")" << RED "FAILED" RESET << std::endl;
    }
}

static void test_incremental_error_detection()
{
    std::cout << "\n=== Testing Error Detection ===" << std::endl;

    HttpRequest req;

    try
    {
        std::string invalidLine = "INVALID /test HTTP/1.1\r\n";
        req.appendAndValidate(invalidLine);
        std::cout << "   This should not be reached!" << std::endl;
    }
    catch (const HttpRequestException &e)
    {
        print_result("Adding invalid method ", e.statusCode() == 405);
    }

    req.reset();

    try
    {
        std::string large_request = "GET ";
        large_request += std::string(5000, 's');
        large_request += " HTTP/1.1\r\n";
        req.appendAndValidate(large_request);
        std::cout << "   This should not be reached!" << std::endl;
    }
    catch (const HttpRequestException &e)
    {
        print_result("Adding too large start line ", e.statusCode() == 414);
    }
}

static void test_http10_getWithoutHost()
{
    std::string raw = "GET /index.html HTTP/1.0" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.0 GET without Host", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.0 GET without Host", false);
    }
}

static void test_http10_getWithHost()
{
    std::string raw = "GET /index.html HTTP/1.0" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.0 GET with Host", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.0 GET with Host", false);
    }
}

static void test_http10_postWithContentLength()
{
    std::string raw = "POST /submit HTTP/1.0" CRLF "Content-Length: 5" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.0 POST with Content-Length", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.0 POST with Content-Length", false);
    }
}

static void test_http10_postWithoutContentLength()
{
    std::string raw = "POST /submit HTTP/1.0" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.0 POST without Content-Length", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.0 POST without Content-Length", ex.statusCode() == 400);
    }
}
static void test_http10_withTransferEncoding()
{
    std::string raw = "POST /submit HTTP/1.0" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.0 with Transfer-Encoding", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.0 with Transfer-Encoding", ex.statusCode() == 400);
    }
}

static void test_http10_compliance()
{
    std::cout << "\n=== Testing HTTP/1.0 Compliance ===" << std::endl;

    test_http10_getWithoutHost();
    test_http10_getWithHost();
    test_http10_postWithContentLength();
    test_http10_postWithoutContentLength();
    test_http10_withTransferEncoding();
}

static void test_http11_getWithoutHost()
{
    std::string raw = "GET /index.html HTTP/1.1" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 GET without Host", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 GET without Host", ex.statusCode() == 400);
    }
}

static void test_http11_getWithHost()
{
    std::string raw = "GET /index.html HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 GET with Host", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 GET with Host", false);
    }
}

static void test_http11_postWithContentLength()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 5" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 POST with Content-Length", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 POST with Content-Length", false);
    }
}

static void test_http11_postWithTransferEncoding()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 POST with Transfer-Encoding", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 POST with Transfer-Encoding", false);
    }
}

static void test_http11_postWithoutLengthInfo()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 POST without length info", !req.isValid() && req.getStatusCode() == 411);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 POST without length info", ex.statusCode() == 411);
    }
}

static void test_http11_compliance()
{
    std::cout << "\n=== Testing HTTP/1.1 Compliance ===" << std::endl;

    test_http11_getWithoutHost();
    test_http11_getWithHost();
    test_http11_postWithContentLength();
    test_http11_postWithTransferEncoding();
    test_http11_postWithoutLengthInfo();
}

static void test_conflicting_headers()
{
    std::cout << "\n=== Testing Conflicting Headers ===" << std::endl;

    {
        std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                          "Content-Length: 5" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
        HttpRequest req;
        std::string buffer = raw;
        try
        {
            req.appendAndValidate(buffer);
            print_result("Content-Length + Transfer-Encoding conflict", !req.isValid() && req.getStatusCode() == 400);
        }
        catch (const HttpRequestException &ex)
        {
            print_result("Content-Length + Transfer-Encoding conflict", ex.statusCode() == 400);
        }
    }
}

static void test_duplicate_headers()
{
    std::cout << "\n=== Testing Duplicate Headers ===" << std::endl;

    {
        std::string raw = "GET /index.html HTTP/1.1" CRLF "Host: localhost" CRLF
                          "Content-Length: 30" CRLF "Content-Length: 20" CRLF CRLF;
        HttpRequest req;
        std::string buffer = raw;
        try
        {
            req.appendAndValidate(buffer);
            print_result("Duplicate headers handling", false);
        }
        catch (const HttpRequestException &ex)
        {
            print_result("Duplicate headers handling", ex.statusCode() == 400);
        }
    }
}

static void test_http11_postWithInvalidLength()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: invalid" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Invalid Content-Length value", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Invalid Content-Length value", ex.statusCode() == 400);
    }
}

static void test_http11_postWithEmptyLength()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: " CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Empty Content-Length value", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Empty Content-Length value", ex.statusCode() == 400);
    }
}

static void test_http11_postWithNegativeLength()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: -5" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Negative Content-Length value", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Negative Content-Length value", ex.statusCode() == 400);
    }
}

static void test_invalid_header_values()
{
    std::cout << "\n=== Testing Invalid Header Values ===" << std::endl;

    test_http11_postWithInvalidLength();
    test_http11_postWithEmptyLength();
    test_http11_postWithNegativeLength();
}

static void test_http10_postWithValidLengthNoBody()
{
    std::string raw = "POST /submit HTTP/1.0" CRLF "Content-Length: 123" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.0 valid Content-Length", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.0 valid Content-Length", false);
    }
}

static void test_http11_postWithValidLengthNoBody()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 456" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 valid Content-Length", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 valid Content-Length", false);
    }
}

static void test_http11_postWithTransferEncodingMany()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Transfer-Encoding: gzip, chunked" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 Transfer-Encoding with chunked", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 Transfer-Encoding with chunked", false);
    }
}

static void test_version_specific_requirements()
{
    std::cout << "\n=== Testing Version-Specific Requirements ===" << std::endl;

    test_http10_postWithValidLengthNoBody();
    test_http11_postWithValidLengthNoBody();
    test_http11_postWithTransferEncodingMany();
}

static void test_http10_postWithLengthZero()
{
    std::string raw = "POST /submit HTTP/1.0" CRLF "Content-Length: 0" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.0 POST Content-Length: 0", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.0 POST Content-Length: 0", false);
    }
}

static void test_http11_postWithLengthZero()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 0" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("HTTP/1.1 POST Content-Length: 0", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("HTTP/1.1 POST Content-Length: 0", false);
    }
}

static void test_http11_postWithLengthTrimmed()
{
    std::string raw = "POST /submit HTTP/1.0" CRLF "Content-Length:  123  " CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with spaces", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with spaces", false);
    }
}

static void test_http11_postWithChunkedCase()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Transfer-Encoding: CHUNKED" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Transfer-Encoding case insensitive", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Transfer-Encoding case insensitive", false);
    }
}

static void test_http11_postLargeLength()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 999999999" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Large Content-Length value", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Large Content-Length value", false);
    }
}

static void test_http11_postWithAlphaLength()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 123abc" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with alphanumeric", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with alphanumeric", ex.statusCode() == 400);
    }
}

static void test_content_length_overflow()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 18446744073709551615" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length overflow", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length overflow", ex.statusCode() == 400);
    }
}

static void test_content_length_negative()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: -5" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Negative Content-Length", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Negative Content-Length", ex.statusCode() == 400);
    }
}

static void test_content_length_leading_zeros()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 00005" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with leading zeros", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with leading zeros", false);
    }
}

static void test_content_length_whitespace_prefix()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length:  \t 5" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with leading whitespace", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with leading whitespace", false);
    }
}

static void test_content_length_whitespace_suffix()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 5 \t " CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with trailing whitespace", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with trailing whitespace", false);
    }
}

static void test_content_length_mixed_whitespace()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: \t 5 \t " CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with mixed whitespace", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with mixed whitespace", false);
    }
}

static void test_content_length_plus_sign()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: +5" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with plus sign", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with plus sign", ex.statusCode() == 400);
    }
}

static void test_content_length_hex()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 0x5" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length hex format", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length hex format", ex.statusCode() == 400);
    }
}

static void test_content_length_octal()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 077" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length octal format", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length octal format", false);
    }
}

static void test_content_length_scientific()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 5e2" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length scientific notation", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length scientific notation", ex.statusCode() == 400);
    }
}

static void test_content_length_floating_point()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 5.0" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length floating point", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length floating point", ex.statusCode() == 400);
    }
}

static void test_content_length_extremely_large()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 999999999999999999999999999999" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Extremely large Content-Length", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Extremely large Content-Length", ex.statusCode() == 400);
    }
}

static void test_content_length_special_chars()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 5#comment" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with special chars", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with special chars", ex.statusCode() == 400);
    }
}

static void test_content_length_unicode()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 5ა" CRLF CRLF;
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Content-Length with unicode", !req.isValid() && req.getStatusCode() == 400);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Content-Length with unicode", ex.statusCode() == 400);
    }
}

static void test_content_length_very_long_digits()
{
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF
                      "Content-Length: 000000000000000000000000000000000000000000005" CRLF CRLF "hello";
    HttpRequest req;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        print_result("Very long digit string", req.isValid() && req.getStatusCode() == 200);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Very long digit string", false);
    }
}

static void test_contentLength1MLimit()
{
    HttpRequest req;
    std::string raw1 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 1048576" CRLF CRLF;
    req.appendAndValidate(raw1);
    try
    {
        req.validateContentLengthLimit(1048576);
        print_result("1M Content-Length within 1M limit", true);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("1M Content-Length within 1M limit", false);
    }
}

static void test_contentLengthExceed1MLimit()
{
    HttpRequest req;
    std::string raw2 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 1048577" CRLF CRLF;
    req.appendAndValidate(raw2);
    try
    {
        req.validateContentLengthLimit(1048576);
        print_result("Exceed 1M limit (413 expected)", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Exceed 1M limit (413 expected)", ex.statusCode() == 413);
    }
}

static void test_zeroContentLengthWithZeroLimit()
{
    HttpRequest req;
    std::string raw3 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 0" CRLF CRLF;
    req.appendAndValidate(raw3);
    try
    {
        req.validateContentLengthLimit(0);
        print_result("Zero Content-Length with zero limit", true);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Zero Content-Length with zero limit", false);
    }
}

static void test_nonZeroContentLengthWithZeroLimit()
{
    HttpRequest req;
    std::string raw4 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 1" CRLF CRLF;
    req.appendAndValidate(raw4);
    try
    {
        req.validateContentLengthLimit(0);
        print_result("Non-zero with zero limit (413 expected)", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Non-zero with zero limit (413 expected)", ex.statusCode() == 413);
    }
}

static void test_largeContentLengthWithLargeLimit()
{
    HttpRequest req;
    std::string raw5 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 1073741824" CRLF CRLF;
    req.appendAndValidate(raw5);
    try
    {
        req.validateContentLengthLimit(1073741824);
        print_result("1G Content-Length within 1G limit", true);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("1G Content-Length within 1G limit", false);
    }
}

static void test_exactlyAtLimitBoundary()
{
    HttpRequest req;
    std::string raw6 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 2048" CRLF CRLF;
    req.appendAndValidate(raw6);
    try
    {
        req.validateContentLengthLimit(2048);
        print_result("Exactly at limit boundary", true);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Exactly at limit boundary", false);
    }
}

static void test_oneByteOverLimit()
{
    HttpRequest req;
    std::string raw7 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 2049" CRLF CRLF;
    req.appendAndValidate(raw7);
    try
    {
        req.validateContentLengthLimit(2048);
        print_result("One byte over limit (413 expected)", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("One byte over limit (413 expected)", ex.statusCode() == 413);
    }
}

static void test_getWithContentLengthOverLimit()
{
    HttpRequest req;
    std::string raw8 = "GET /index.html HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 100" CRLF CRLF;
    req.appendAndValidate(raw8);
    try
    {
        req.validateContentLengthLimit(50);
        print_result("GET with Content-Length over limit (413 expected)", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("GET with Content-Length over limit (413 expected)", ex.statusCode() == 413);
    }
}

static void test_deleteWithContentLengthOverLimit()
{
    HttpRequest req;
    std::string raw9 = "DELETE /item HTTP/1.1" CRLF "Host: localhost" CRLF "Content-Length: 100" CRLF CRLF;
    req.appendAndValidate(raw9);
    try
    {
        req.validateContentLengthLimit(50);
        print_result("DELETE with Content-Length over limit (413 expected)", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("DELETE with Content-Length over limit (413 expected)", ex.statusCode() == 413);
    }
}

static void test_postWitoutContentLength()
{
    HttpRequest req;
    std::string raw10 = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF CRLF;
    try
    {
        req.appendAndValidate(raw10);
        req.validateContentLengthLimit(1000);
        print_result("POST without Content-Length (no validation)", true);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("POST without Content-Length (parsing exception expected)", ex.statusCode() == 411 || ex.statusCode() == 400);
    }
}

static void test_chunk_basic_hex_numbers()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF "5\r\nHello\r\n";
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        int size = req.validateChunkSize(buffer);
        bool pass = req.isValid() && size == 5;
        print_result("Chunk basic hex numbers", pass);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk basic hex numbers", false);
    }
}

static void test_chunk_mixed_case_hex()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF "AbC\r\ntest\r\n";
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        int size = req.validateChunkSize(buffer);
        bool pass = req.isValid() && size == 2748; // ABC ==> 2748
        print_result("Chunk mixed case hex", pass);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk mixed case hex", false);
    }
}

static void test_chunk_extensions()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF "5;name=value\r\nHello\r\n";
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        int size = req.validateChunkSize(buffer);
        (void)size;
        bool pass = !req.isValid();
        print_result("Chunk with extensions", pass);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk with extensions (exception)", true);
    }
}

static void test_chunk_quoted_extensions()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF "B;name=\"quoted value\"\r\ntest\r\n";
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        int size = req.validateChunkSize(buffer);
        (void)size;
        bool pass = !req.isValid();
        print_result("Chunk with quoted extensions", pass);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk with quoted extensions (exception)", true);
    }
}

static void test_chunk_empty_buffer()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string empty_buffer = "";
        req.validateChunkSize(empty_buffer);
        print_result("Chunk empty buffer", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk empty buffer", ex.statusCode() == 400);
    }
}

static void test_chunk_missing_crlf()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "5Hello";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk missing CRLF", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk missing CRLF", ex.statusCode() == 400);
    }
}

static void test_chunk_empty_size_line()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "\r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk empty size line", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk empty size line", ex.statusCode() == 400);
    }
}

static void test_chunk_invalid_hex_chars()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "5G\r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk invalid hex characters", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk invalid hex characters", ex.statusCode() == 400);
    }
}

static void test_chunk_no_hex_digits()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "xyz\r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk no hex digits at start", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk no hex digits at start", ex.statusCode() == 400);
    }
}

static void test_chunk_size_too_large()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "123456789\r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk size too large", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk size too large", ex.statusCode() == 413);
    }
}

static void test_chunk_invalid_extension()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "5; \r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk invalid extension", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk invalid extension", ex.statusCode() == 400);
    }
}

static void test_chunk_unterminated_quoted_string()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "5;name=\"unterminated\r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk unterminated quoted string", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk unterminated quoted string", ex.statusCode() == 400);
    }
}

static void test_chunk_invalid_chars_after_size()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "5xyz\r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk invalid chars after size", false);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk invalid chars after size", ex.statusCode() == 400);
    }
}

static void test_chunk_whitespace_handling()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer1 = "5 \r\ndata";
        int size1 = req.validateChunkSize(chunk_buffer1);

        bool pass = !req.isValid() && size1 == -1;
        print_result("Chunk whitespace handling", pass);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk whitespace handling (exception)", true);
    }
}

static void test_chunk_max_size()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "FFFFFFFF\r\ndata";
        req.validateChunkSize(chunk_buffer);
        print_result("Chunk maximum size", req.isValid());
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk maximum size (overflow protection)", ex.statusCode() == 413);
    }
}

static void test_chunk_zero_size()
{
    HttpRequest req;
    std::string raw = "POST /submit HTTP/1.1" CRLF "Host: localhost" CRLF "Transfer-Encoding: chunked" CRLF CRLF;
    std::string buffer = raw;
    try
    {
        req.appendAndValidate(buffer);
        std::string chunk_buffer = "0\r\n\r\n";
        int size = req.validateChunkSize(chunk_buffer);
        bool pass = req.isValid() && size == 0;
        print_result("Chunk zero size", pass);
    }
    catch (const HttpRequestException &ex)
    {
        print_result("Chunk zero size", false);
    }
}

static void test_chunk_validation()
{
    std::cout << "\n=== Chunk Size Validation Tests ===" << std::endl;

    test_chunk_basic_hex_numbers();
    test_chunk_mixed_case_hex();
    test_chunk_extensions();
    test_chunk_quoted_extensions();
    test_chunk_empty_buffer();
    test_chunk_missing_crlf();
    test_chunk_empty_size_line();
    test_chunk_invalid_hex_chars();
    test_chunk_no_hex_digits();
    test_chunk_size_too_large();
    test_chunk_invalid_extension();
    test_chunk_unterminated_quoted_string();
    test_chunk_invalid_chars_after_size();
    test_chunk_whitespace_handling();
    test_chunk_max_size();
    test_chunk_zero_size();
}

static void test_body_size_limit_validation()
{
    std::cout << "\n=== Content-Length vs Body Size Limit Tests ===\n";

    test_contentLength1MLimit();
    test_contentLengthExceed1MLimit();
    test_zeroContentLengthWithZeroLimit();
    test_nonZeroContentLengthWithZeroLimit();
    test_largeContentLengthWithLargeLimit();
    test_exactlyAtLimitBoundary();
    test_oneByteOverLimit();
    test_getWithContentLengthOverLimit();
    test_deleteWithContentLengthOverLimit();
    test_postWitoutContentLength();
}

static void test_content_length_rfc_compliance()
{
    std::cout << "\n=== Content-Length RFC Compliance Tests ===\n";
    test_content_length_overflow();
    test_content_length_negative();
    test_content_length_leading_zeros();
    test_content_length_whitespace_prefix();
    test_content_length_whitespace_suffix();
    test_content_length_mixed_whitespace();
    test_content_length_plus_sign();
    test_content_length_hex();
    test_content_length_octal();
    test_content_length_scientific();
    test_content_length_floating_point();
    test_content_length_extremely_large();
    test_content_length_special_chars();
    test_content_length_unicode();
    test_content_length_very_long_digits();
}

static void test_edge_cases()
{
    std::cout << "\n=== Testing Edge Cases ===" << std::endl;

    test_http10_postWithLengthZero();
    test_http11_postWithLengthZero();
    test_http11_postWithLengthTrimmed();
    test_http11_postWithChunkedCase();
    test_http11_postLargeLength();
    test_http11_postWithAlphaLength();
}

void testHttpRequestParse()
{
    std::cout << "\n=== HttpRequest Parse Tests ===\n";
    test_get_valid();
    test_post_valid();
    test_post_valid_no_body();
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
    test_cookie_parsing();
    test_session_id();
    test_cookie_edge_cases();

    test_incremental_parsing();
    test_incremental_error_detection();

    test_http10_compliance();
    test_http11_compliance();
    test_conflicting_headers();
    test_duplicate_headers();
    test_invalid_header_values();
    test_version_specific_requirements();

    test_body_size_limit_validation();
    test_content_length_rfc_compliance();

    test_edge_cases();
    test_chunk_validation();

    std::cout << "=== End HttpRequest Parse Tests ===\n";
}