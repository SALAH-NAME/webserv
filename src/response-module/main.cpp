#include <string>
#include <iostream>

int main()
{
    std::string response_body = "<html>\n<head><title>301 Moved Permanently"
        "</title></head>\n<body>\n<center><h1>301 Moved Permanently"
        "</h1></center>\n<hr><center>nginx/1.18.0 (Ubuntu)</center>\n"
        "</body>\n</html>"; 
    std::cout << response_body;
}