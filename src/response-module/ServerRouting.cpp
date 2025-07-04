#include "ResponseHandler.hpp"

bool ResponseHandler::NeedToRedirect(Request &req){
    return (IsDir(resource_path.c_str()) || loc_config->getRedirect().isValid());
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
            if (IsDir((it->second.getRoot()+req_path).c_str()))//if the path exist but as a directory
                throw (RequestError("HTTP/1.1 403 Forbidden", 403));
            resource_path = it->second.getRoot() + '/' + req_path;
            loc_config = &it->second;
            require_cgi = true;
            return (true);
        }
    }
}

bool locationMatched(const std::string &req_path, const LocationConfig &locationConf,
        std::string &current_path, const std::string &method)
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

void ResponseHandler::RouteResolver(const std::string &req_path, const std::string &method)
{
    LOCATIONS   &srv_locations = conf.getLocations();
    if (method != "DELETE" && CheckForCgi(req_path, srv_locations))
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
        throw (RequestError("HTTP/1.1 404 Not Found", 404));//the request path didn't match with any location
}
