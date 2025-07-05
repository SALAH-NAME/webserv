#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler(int sockfd, ServerConfig &server_conf) : conf(server_conf)
{
    InitializeStandardContentTypes();
    socket_fd = sockfd;
    loc_config = NULL;
    resource_path = "";
    require_cgi = false;
    response_file = new File();
}

std::string	ResponseHandler::GetResponseHeader(){return response_header;}

std::string ResponseHandler::GetResponseBody(){return response_body;}

File *ResponseHandler::GetResponseFilePtr(){return response_file;}

int *ResponseHandler::GetCgiInPipe(){return (CgiObj.GetInPipe());}

int *ResponseHandler::GetCgiOutPipe(){return (CgiObj.GetOutPipe());}

pid_t ResponseHandler::GetCgiChildPid(){return (CgiObj.GetChildPid());}

void ResponseHandler::Run(Request &req)
{
    response_header = "";
    response_body = "";
    resource_path = "";
    require_cgi = false;
    loc_config = NULL;
    response_file = new File();
    try {
        ProccessRequest(req);
    }
    catch (ResponseHandlerError &ex)
    {
        LoadErrorPage(ex.what(), ex.getStatusCode(), req);
    }
}

void ResponseHandler::ProccessRequest(Request &req)
{
    CheckForInitialErrors(req);
    RouteResolver(req.getPath(), req.getMethod());//  set the resource_path and loc_config
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
            ProccessHttpDELETE(req);
            break;
    }
}

void ResponseHandler::ProccessHttpGET(Request &req)
{
    if (require_cgi)
        return (CgiObj.RunCgi(req, conf, *loc_config, resource_path));
    if (!access(resource_path.c_str(), R_OK) || (IsDir(resource_path.c_str())
        && loc_config->getIndex().empty() && !loc_config->getAutoindex()))
            throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
    if (IsDir(resource_path.c_str()))
    {
        if (loc_config->getIndex().empty())
            return (GenerateDirListing(req));
        else
            return (LoadStaticFile(req, loc_config->getIndex().front()));
    }
    return (LoadStaticFile(req, resource_path));
}

void ResponseHandler::ProccessHttpPOST(Request &req)
{
    if (require_cgi)
        CgiObj.RunCgi(req, conf, *loc_config, resource_path);
    if (access(resource_path.c_str(), F_OK))
        throw ("HTTP/1.1 409 Conflict", 409);
    if (req.getPath()[req.getPath().size() - 1] == '/')
        throw ("HTTP/1.1 403 Forbidden", 403);

    //create the file with the same name as the full path and use the body of the request as it's content
}

void ResponseHandler::ProccessHttpDELETE(Request &req)
{
    if (!access(resource_path.c_str(), R_OK) || IsDir(resource_path.c_str()))
        throw("HTTP/1.1 403 Forbidden");
    if (std::remove(resource_path.c_str()) == -1)
        throw("HTTP/1.1 500 Internal Server Error");
    SetResponseHeader(req, "HTTP/1.1 200 OK", 0);
}

ResponseHandler::~ResponseHandler(){
    delete response_file;
}
