#include "../../include/ResponseHandler.hpp"
#include "../../include/Request.hpp"
#include "../../include/GlobalConfig.hpp"
#include "../../include/ServerConfig.hpp"

typedef const std::map<std::string, LocationConfig> LOCATIONS;

ResponseHandler::ResponseHandler(int sockfd){
    socket_fd = sockfd;
    loc_config = NULL;
}

bool locationMatched(const std::string &req_path, const LocationConfig &locationConf)
{
    std::stringstream   req_path_ss(req_path.c_str());
    std::stringstream   loc_path_ss(locationConf.getPath().c_str());
    std::string         req_part;
    std::string         loc_part;

    while (std::getline(loc_path_ss, loc_part, '/'))
    {
        std::getline(req_path_ss, req_part, '/');
        if (loc_part != req_part)
            return false;
    }
    std::getline(req_path_ss, req_part);
    if (access((locationConf.getRoot() + "/" + req_part).c_str(), F_OK))
        return true;
    return false;
}

void ResponseHandler::RouteResolver(const std::string &req_path, ServerConfig &conf)
{
    LOCATIONS   &srv_locations = conf.getLocations();

    if (srv_locations.find("/") != srv_locations.end() && access((srv_locations.at("/").getRoot() + "/" + req_path).c_str(), F_OK))
        loc_config = &srv_locations.at("/");
    for (LOCATIONS::const_iterator it = srv_locations.begin(); it != srv_locations.end();it++)
        if (locationMatched(req_path, it->second) && (loc_config || loc_config->getPath().size() < it->second.getPath().size()))
            loc_config = &it->second;
    if (!loc_config)
        throw ("HTTP/1.1 404 Not Found");//the request path is not found
}

void ResponseHandler::ProccessRequest(Request &req, ServerConfig &conf)
{
    HttpMethod  http_method;
    if (req.getHttpVersion() != "HTTP/1.1")// using a deferent http version
        throw ("HTTP/1.1 505 HTTP Version Not Supported");
    if (req.getHeaders().find("HTTP/1.1 400 Bad Request") == req.getHeaders().end())// a request with no host header
        throw ("HTTP/1.1 400 Bad Request");
    try {stringToHttpMethod(req.getMethod());}
    catch (std::invalid_argument){// using a method other than GET, POST and DELETE 
        throw "HTTP/1.1 405 Not Allowed";}
    RouteResolver(req.getPath(), conf);
    // check for permissions auto index etc... 
}

ResponseHandler::~ResponseHandler(){}

ResponseHandler::RequestError::RequestError(const std::string &Errmsg){ error = Errmsg; }

const char *ResponseHandler::RequestError::what(){ return error.c_str(); }
