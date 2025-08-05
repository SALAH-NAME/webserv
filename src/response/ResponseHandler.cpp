#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler(const ClientInfos clientInfos, const ServerConfig &server_conf) : conf(server_conf), target_file(NULL)
{
    InitializeStandardContentTypes();
    InitializeStatusPhrases();
    client_info = clientInfos;
    loc_config = NULL;
    is_location_allocated = false;
    resource_path = "";
    require_cgi = false;
    is_post = false;
    post_failed = false;
}

std::string	ResponseHandler::GetResponseHeader(){return response_header;}

std::string ResponseHandler::GetResponseBody(){return response_body;}

std::fstream *ResponseHandler::GetTargetFilePtr(){return target_file;}

bool ResponseHandler::IsPost(){return is_post;}

std::string ResponseHandler::GetResourcePath() {return resource_path;}

bool ResponseHandler::postFailed() {return post_failed;}

void ResponseHandler::Run(HttpRequest &req)
{
    response_header = "";
    response_body = "";
    resource_path = "";
    require_cgi = false;
    is_post = false;
    post_failed = false;
    cgi_buffer_size = 0;
    loc_config = NULL;
    if (is_location_allocated){
        delete loc_config;
        loc_config = NULL;
        is_location_allocated = false;
    }
    if (target_file){
        target_file->close();
        delete target_file;
        target_file = NULL;
    }
    try {
        ProccessRequest(req);
    }
    catch (ResponseHandlerError &ex)
    {
        LoadErrorPage(ex.what(), ex.getStatusCode());
    }
}

void ResponseHandler::ProccessRequest(HttpRequest &req)
{
    InitialRequestCheck(req);
    RouteResolver(req.getPath(), req.getMethod());//  set resource_path and loc_config
    if (NeedToRedirect(req))
        return (GenerateRedirection(req));
    if (loc_config->getAllowedMethods().find(stringToHttpMethod(req.getMethod())) == loc_config->getAllowedMethods().end())
        throw (ResponseHandlerError("HTTP/1.1 405 Not Allowed", 405));//req method is not allowed on the route
    switch (stringToHttpMethod(req.getMethod()))
    {
        case HTTP_GET:
            ProccessHttpGET(req);
            break;
        case HTTP_POST:
            ProccessHttpPOST(req);
            break;
        case HTTP_DELETE:
            ProccessHttpDELETE();
            break;
    }
}

void    ResponseHandler::HandleDirRequest(HttpRequest &req)
{
    const std::vector<std::string> &indexes = loc_config->getIndex();
    std::string current_path;
    if (indexes.empty())
        return (GenerateDirListing(req));
    for (unsigned int i=0;i<indexes.size();i++)
    {
        current_path = resource_path + '/' + indexes[i];
        if (access(current_path.c_str(), R_OK) == 0)
            return (LoadStaticFile(current_path));
    }
    if (loc_config->getAutoindex())
        return (GenerateDirListing(req));
    else 
        throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
}

void ResponseHandler::ProccessHttpGET(HttpRequest &req)
{
    if (require_cgi)
        return (CgiObj.RunCgi(req, conf, *loc_config, resource_path, client_info));    
    if (access(resource_path.c_str(), R_OK) != 0 || (IsDir(resource_path.c_str())
        && loc_config->getIndex().empty() && !loc_config->getAutoindex()))
            throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
    if (IsDir(resource_path.c_str()))
        return (HandleDirRequest(req));
    return (LoadStaticFile(resource_path));
}

void ResponseHandler::ProccessHttpPOST(HttpRequest &req)
{
    if (require_cgi)
        return (CgiObj.RunCgi(req, conf, *loc_config, resource_path, client_info));
    if (access(resource_path.c_str(), F_OK) == 0){
        post_failed = true;
        throw (ResponseHandlerError("HTTP/1.1 409 Conflict", 409));}
    if (access(GetFileDirectoryPath(resource_path).c_str(), W_OK | X_OK) != 0 ||
            req.getPath()[req.getPath().size() - 1] == '/'){
        post_failed = true;
        throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
    }
    SetResponseHeader("HTTP/1.1 201 Created", -1, false);
    target_file = new std::fstream(resource_path.c_str(), std::ios::out);
    if (!target_file || !target_file->is_open())
        throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
    is_post = true;
}

void ResponseHandler::ProccessHttpDELETE()
{
    if (access(resource_path.c_str(), R_OK) != 0 || IsDir(resource_path.c_str()))
        throw(ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
    if (std::remove(resource_path.c_str()) == -1)
        throw(ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
    SetResponseHeader("HTTP/1.1 200 OK", -1, false);
}

ResponseHandler::~ResponseHandler(){
	if (target_file) {
		target_file->close();
		delete target_file;
		target_file = NULL;
	}
}
