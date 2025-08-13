#include "ResponseHandler.hpp"

bool ResponseHandler::NeedToRedirect(){
    return ((IsDir(resource_path.c_str()) &&
        req->getPath()[req->getPath().size()-1] != '/') ||
            loc_config->hasRedirect());
}

ServerConfig* getMatchingServerConfig(const std::vector<ServerConfig>& configs, std::string host) {
	int defaultIndex = -1;

    if (host.empty())
        return &(const_cast<ServerConfig&>(configs[0]));

    for (size_t i = 0; i < configs.size(); ++i) {
		if (defaultIndex == -1)
			defaultIndex = i;

        const std::vector<std::string>& serverNames = configs[i].getServerNames();
        for (size_t j = 0; j < serverNames.size(); ++j) {
            if (serverNames[j] == host) {
                return &(const_cast<ServerConfig&>(configs[i]));
            }
        }
    }

    // Fallback: return first config as default
	if (defaultIndex == -1)
		defaultIndex = 0;
    return &(const_cast<ServerConfig&>(configs[defaultIndex]));
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
            if (access((it->second.getRoot() + req_path).c_str(), F_OK) != 0)
                throw (ResponseHandlerError(req->getVersion() + " 404 Not Found", 404));
            if (IsDir((it->second.getRoot()+req_path).c_str()))//if the path exist but as a directory
                throw (ResponseHandlerError(req->getVersion() + " 403 Forbidden", 403));
            resource_path = it->second.getRoot() + '/' + req_path;
            loc_config = &it->second;
            require_cgi = true;
            return (true);
        }
    }
    return (false);
}

std::string GetRestOfPath(const std::string &full_path, int pos)
{
    unsigned int i = 0;
    while (pos > 0)
    {
        while (full_path[i] == '/' && i < full_path.size())
            i++;
        while (i < full_path.size())
        {
            if (full_path[i] == '/')
                break;
            i++;
        }
        pos--;
    }
    return full_path.substr(i);
}

bool PathPartExtractor(const std::string &full_path, int current_pos, std::string &part)
{
	bool found = false;
    unsigned int i = 0;
    while (current_pos >= 0)
    {
		part = "";
		found = false;
        while (full_path[i] == '/' && i < full_path.size())
            i++;
        for (;i < full_path.size();i++)
        {
            if (full_path[i] != '/'){
                part += full_path[i];
				found = true;
			}
            else
                break ;
        }
        current_pos--;
    }
	return (found);
}

std::string GetFileDirectoryPath(const std::string &path)
{
    int i = path.size() - 1;
    while( i >= 0 && path[i] == '/')
        i--;
    for (;i >= 0; i--)
        if (path[i] == '/')
            return (path.substr(0, i) + '/');
    return "";
}

bool locationMatched(const std::string &req_path, const LocationConfig &locationConf,
        std::string &current_path, const std::string &method)
{
    int                 pos = 0;
    std::string         req_part;
    std::string         testing_path;
    std::string         loc_part;

    while (PathPartExtractor(locationConf.getPath(), pos, loc_part))//  comapiring the location with the request path
    {
        PathPartExtractor(req_path, pos, req_part);
        if (loc_part != req_part)
            return false;//route didn't match with the request path
        pos++;
    }
    if (loc_part.empty() && !PathPartExtractor(req_path, pos, req_part) && locationConf.hasRedirect())
        return true;
    req_part = GetRestOfPath(req_path, pos);
    testing_path = locationConf.getRoot() + "/" + (method != "POST" ? req_part : GetFileDirectoryPath(req_part)); // appending the req_part to the config root if not POST
//     // std::cout << "testing path: " << testing_path << std::endl;//logger
    if (access(testing_path.c_str(), F_OK) == 0){// checks if the resulting path exists
        current_path = locationConf.getRoot() + "/" + req_part;
        return true;
    }
    return false;
}

void ResponseHandler::MakeLocationFromSrvConf()
{
    LocationConfig *tmp = new LocationConfig();
    tmp->setAllowedMethods(conf->getAllowedMethods());
    tmp->setAutoindex(conf->getAutoindex());
    tmp->setClientMaxBodySize(NumtoString(conf->getClientMaxBodySize()));
    tmp->setErrorPages(conf->getErrorPages());
    tmp->setIndex(conf->getIndex());
    tmp->setPath("/");
    tmp->setRoot(conf->getRoot());
    loc_config = tmp;
}

void ResponseHandler::RouteResolver(const std::string &req_path, const std::string &method)
{
    LOCATIONS   &srv_locations = conf->getLocations();
    if (req_path == "/" && srv_locations.find("/") != srv_locations.end())
    {
        resource_path = srv_locations.at("/").getRoot();
        loc_config = &srv_locations.at("/");
        return;
    }
    if (method != "DELETE" && CheckForCgi(req_path, conf->getRegexLocation()))
        return ;
    std::string current_resource_path;//    will be setted by 'locationMatched' each time a route is validated and is longer than prev value
    if (srv_locations.find("/") != srv_locations.end()
            && access((srv_locations.at("/").getRoot() + "/" + req_path).c_str(), F_OK) == 0)
    {
        // std::cout << "inside the root matcher" << std::endl; // logger
        loc_config = &srv_locations.at("/");
        resource_path = srv_locations.at("/").getRoot() + "/" + req_path;
    }
        for (LOCATIONS::const_iterator it = srv_locations.begin(); it != srv_locations.end();it++)
    {
        if (locationMatched(req_path, it->second, current_resource_path, method) &&
            (!loc_config || loc_config->getPath().size() < it->second.getPath().size())){
            loc_config = &it->second;//     update if the new route is longer
            resource_path = current_resource_path;
        }
    }
    if (access((conf->getRoot() + req_path).c_str(), F_OK) == 0){
        MakeLocationFromSrvConf();
        resource_path = conf->getRoot() + req_path;
    }
    if (!loc_config)
        throw (ResponseHandlerError(req->getVersion() + " 404 Not Found", 404));//the request path didn't match with any location
}
