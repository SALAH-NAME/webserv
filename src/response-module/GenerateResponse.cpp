#include "ResponseHandler.hpp"

void    ResponseHandler::GenerateDirListing(Request &req, ServerConfig &conf)
{
    response_body =
    "<html>\n<head><title>Index of " + req.getPath() + "</title></head>\n<body>\
    <h1>Index of " + req.getPath() + "</h1><hr><pre>";

}
//check out : opendir, readdir and closedir.