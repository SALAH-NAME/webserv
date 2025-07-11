#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler(const ServerConfig &server_conf) : conf(server_conf)
{
    InitializeStandardContentTypes();
    loc_config = NULL;
    resource_path = "";
    require_cgi = false;
    is_post = false;
    target_file = new std::fstream();
}

std::string	ResponseHandler::GetResponseHeader(){return response_header;}

std::string ResponseHandler::GetResponseBody(){return response_body;}

std::fstream *ResponseHandler::GetTargetFilePtr(){return target_file;}

bool ResponseHandler::IsPost(){return is_post;}

int *ResponseHandler::GetCgiInPipe(){return (CgiObj.GetInPipe());}

int *ResponseHandler::GetCgiOutPipe(){return (CgiObj.GetOutPipe());}

pid_t ResponseHandler::GetCgiChildPid(){return (CgiObj.GetChildPid());}

void ResponseHandler::Run(Request &req)
{
    response_header = "";
    response_body = "";
    resource_path = "";
    require_cgi = false;
    is_post = false;
    loc_config = NULL;
    delete target_file;
    target_file = new std::fstream();
    try {
        ProccessRequest(req);
    }
    catch (ResponseHandlerError &ex)
    {
        LoadErrorPage(ex.what(), ex.getStatusCode());
    }
}

void ResponseHandler::ProccessRequest(Request &req)
{
    CheckForInitialErrors(req);
    RouteResolver(req.getPath(), req.getMethod());//  set resource_path and loc_config
    std::cout << "log: path= " << resource_path << std::endl;
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

void    ResponseHandler::HandleDirRequest(Request &req)
{
    const std::vector<std::string> &indexes = loc_config->getIndex();
    std::string current_path;
    if (indexes.empty())
        return (GenerateDirListing(req));
    for (unsigned int i=0;i<indexes.size();i++)
    {
        current_path = loc_config->getRoot() + '/' + indexes[i];
        if (access(current_path.c_str(), R_OK) == 0)
            return (LoadStaticFile(current_path));
    }
    if (loc_config->getAutoindex())
        return (GenerateDirListing(req));
    else 
        throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
}

void ResponseHandler::ProccessHttpGET(Request &req)
{
    if (require_cgi)
        return (CgiObj.RunCgi(req, conf, *loc_config, resource_path));    
    if (access(resource_path.c_str(), R_OK) != 0 || (IsDir(resource_path.c_str())
        && loc_config->getIndex().empty() && !loc_config->getAutoindex()))
            throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
    if (IsDir(resource_path.c_str()))
        return (HandleDirRequest(req));
    return (LoadStaticFile(resource_path));
}

void ResponseHandler::ProccessHttpPOST(Request &req)
{
    if (require_cgi)
        CgiObj.RunCgi(req, conf, *loc_config, resource_path);
    if (access(resource_path.c_str(), F_OK) == 0)
        throw (ResponseHandlerError("HTTP/1.1 409 Conflict", 409));
    if (req.getPath()[req.getPath().size() - 1] == '/')
        throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
    SetResponseHeader("HTTP/1.1 200 OK", -1, false);
    target_file->open(resource_path.c_str(), std::ios::out);
    if (!target_file->is_open())
        throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
    is_post = true;
}

void ResponseHandler::ProccessHttpDELETE()
{
    if (access(resource_path.c_str(), R_OK) != 0 || IsDir(resource_path.c_str()))
        throw("HTTP/1.1 403 Forbidden");
    if (std::remove(resource_path.c_str()) == -1)
        throw("HTTP/1.1 500 Internal Server Error");
    SetResponseHeader("HTTP/1.1 200 OK", -1, false);
}

ResponseHandler::~ResponseHandler(){
    delete target_file;
}
