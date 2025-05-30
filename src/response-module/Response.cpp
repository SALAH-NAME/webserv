#include "../../include/Response.hpp"
#include "../../include/Request.hpp"
#include "../../include/GlobalConfig.hpp"

Response::Response(void) {}

Response::Response(int fd) : socket_fd(fd) {}

void    Response::setResponse(std::string responseData) {
    responseHolder += responseData;
}

std::string Response::getResponse(void) {
    return responseHolder;
}

int			Response::getFD() {
    return socket_fd;
}