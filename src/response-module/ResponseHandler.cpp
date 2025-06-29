#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler(int sockfd)
{
    std::string image[] = {"jpeg", "jpg", "png", "svg", "webp", "vnd.microsoft.icon"};
    std::string text[] = {"html", "htm", "css", "javascript", "xml", "csv"};
    std::string audio[] = {"mpeg", "wav"};
    std::string video[] = {"mp4", "webm"};
    std::string application[] = {"json", "pdf", "xml", "zip", "wasm"};
  
    content_types["image/"] = std::vector<std::string>(image, image+6);
    content_types["text/"] = std::vector<std::string>(text, text+6);
    content_types["video/"] = std::vector<std::string>(video, video+2);
    content_types["audio/"] = std::vector<std::string>(audio, audio+2);
    content_types["application/"] = std::vector<std::string>(application, application+5);
    socket_fd = sockfd;
    loc_config = NULL;
    resource_path = "";
    require_cgi = false;
}


int *ResponseHandler::GetCgiInPipe(){return (CgiObj.GetInPipe());}

int *ResponseHandler::GetCgiOutPipe(){return (CgiObj.GetOutPipe());}

pid_t ResponseHandler::GetCgiChildPid(){return (CgiObj.GetChildPid());}

bool locationMatched(const std::string &req_path, const LocationConfig &locationConf, std::string &current_path, const std::string &method)
{
    std::stringstream   req_path_ss(req_path.c_str());
    std::stringstream   loc_path_ss(locationConf.getPath().c_str());
    std::string         req_part;
    std::string         testing_path;
    std::string         loc_part;

    while (std::getline(loc_path_ss, loc_part, '/'))//  comapiring the location with the request path
    {
        std::getline(req_path_ss, req_part, '/');
        if (loc_part != req_part)
            return false;//route didn't match with the request path
    }
    std::getline(req_path_ss, req_part);
    testing_path = locationConf.getRoot() + "/" + (method != "POST" ? req_part : ""); // appending the req_part to the config root if not POST
    if (access(testing_path.c_str(), F_OK)){// checks if the resulting path exists
        current_path = locationConf.getRoot() + "/" + req_part;
        return true;
    }
    return false;
}

bool ResponseHandler::CheckForCgi(const std::string &req_path, LOCATIONS &srv_locations)
{
    std::string extension = ExtractFileExtension(req_path);
    if (extension.empty())
        return (false);
    for (LOCATIONS::const_iterator it = srv_locations.begin(); it != srv_locations.end();it++)
    {
        if (it->second.isCgi() && it->second.getPath() == extension)
        {
            //the requested file extension matched with a cgi location
            if (!access((it->second.getRoot() + req_path).c_str(), F_OK))
                return (false);
            if (is_dir((it->second.getRoot()+req_path).c_str()))//if the path exist but as a directory
                throw(RequestError("HTTP/1.1 403 Forbidden"));
            resource_path = it->second.getRoot() + req_path;
            loc_config = &it->second;
            require_cgi = true;
            return (true);
        }
    }
}

void ResponseHandler::RouteResolver(const std::string &req_path, ServerConfig &conf, const std::string &method)
{
    LOCATIONS   &srv_locations = conf.getLocations();
    if (CheckForCgi(req_path, srv_locations) && method != "DELETE")
        return ;
    std::string current_resource_path;//    will be setted by 'locationMatched' each time a route is validated and is longer than prev value
    if (srv_locations.find("/") != srv_locations.end() && access((srv_locations.at("/").getRoot() + "/" + req_path).c_str(), F_OK))
        loc_config = &srv_locations.at("/");//   if the path matches with the '/' location the full path will be used (it may be changed later in the code)
    for (LOCATIONS::const_iterator it = srv_locations.begin(); it != srv_locations.end();it++)
    {
        if (locationMatched(req_path, it->second, current_resource_path, method) &&
            (loc_config || loc_config->getPath().size() < it->second.getPath().size())){
            loc_config = &it->second;//     update if the new route is longer
            resource_path = current_resource_path;
        }
    }
    if (!loc_config)
        throw (RequestError("HTTP/1.1 404 Not Found"));//the request path didn't match with any location
}

void ResponseHandler::ProccessRequest(Request &req, ServerConfig &conf)
{
    if (req.getHttpVersion() != "HTTP/1.1")// using a different http version
        throw (RequestError("HTTP/1.1 505 HTTP Version Not Supported"));
    if (req.getHeaders().find("Host") == req.getHeaders().end())// a request with no host header
        throw (RequestError("HTTP/1.1 400 Bad Request"));
    try {stringToHttpMethod(req.getMethod());}
    catch (std::invalid_argument){//    using a method other than GET, POST and DELETE 
        throw (RequestError("HTTP/1.1 405 Not Allowed"));}
    RouteResolver(req.getPath(), conf, req.getMethod());//  set the resource_path and loc_config methods to the appropriate value (full route & location conf)
    if (loc_config->getAllowedMethods().find(stringToHttpMethod(req.getMethod())) == loc_config->getAllowedMethods().end())
        throw (RequestError("HTTP/1.1 405 Not Allowed"));//req method is not allowed on the route
    switch (stringToHttpMethod(req.getMethod()))
    {
        case HTTP_GET:
            ProccessHttpGET(req, conf);
            break;
        case HTTP_POST:
            ProccessHttpPOST(req, conf);
            break;    
        case HTTP_DELETE:
            ProccessHttpDELETE(req, conf);
            break;
    }
}

void ResponseHandler::SetResponseHeader(Request &req, ServerConfig &conf, const std::string &status_line)
{
    struct stat path_info;

    if (stat(resource_path.c_str(), &path_info) != 0)//if it failed -> server error because file does exist with the right permissions
        throw (RequestError("HTTP/1.1 500 Internal Server Error"));
    response_header = status_line + CRLF + "server: " + SRV_NAME + CRLF + "Date: " +
        GenerateTimeStamp() + CRLF ;
    response_header += (req.getMethod() == "GET") ? GenerateContentType(ExtractFileExtension(resource_path))
        + CRLF + "Content-Length: " + NumtoString(path_info.st_size) : "";
    
}

void ResponseHandler::ProccessHttpGET(Request &req, ServerConfig &conf)
{
    if (require_cgi)
        CgiObj.RunCgi(req, conf, *loc_config, resource_path);
    if (!access(resource_path.c_str(), R_OK) || (is_dir(resource_path.c_str())
        && loc_config->getIndex().empty() && !loc_config->getAutoindex()))
            throw (RequestError("HTTP/1.1 403 Forbidden"));
    if (loc_config->getIndex().empty() && is_dir(resource_path.c_str()))
        GenerateDirListing(req, conf);
}

void ResponseHandler::ProccessHttpPOST(Request &req, ServerConfig &conf)
{
    if (require_cgi)
        CgiObj.RunCgi(req, conf, *loc_config, resource_path);
    if (access(resource_path.c_str(), F_OK))
        throw ("HTTP/1.1 409 Conflict");
    if (req.getPath()[req.getPath().size() - 1] == '/')
        throw ("HTTP/1.1 403 Forbidden");

    //create the file with the same name as the full path and use the body of the request as it's content
}

void ResponseHandler::ProccessHttpDELETE(Request &req, ServerConfig &conf)
{
    if (!access(resource_path.c_str(), R_OK) || is_dir(resource_path.c_str()))
        throw("HTTP/1.1 403 Forbidden");
    if (std::remove(resource_path.c_str()) == -1)
        throw("HTTP/1.1 500 Internal Server Error");
}


ResponseHandler::~ResponseHandler(){}

ResponseHandler::RequestError::RequestError(const std::string &Errmsg){ error = Errmsg; }

const char *ResponseHandler::RequestError::what(){return error.c_str();}
