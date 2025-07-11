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

void ResponseHandler::Run(HttpRequest &req)
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
        LoadErrorPage(ex.what(), ex.getStatusCode(), req);
    }
}

void ResponseHandler::ProccessRequest(HttpRequest &req)
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

void ResponseHandler::ProccessHttpGET(HttpRequest &req)
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

void ResponseHandler::ProccessHttpPOST(HttpRequest &req)
{
    if (require_cgi)
        CgiObj.RunCgi(req, conf, *loc_config, resource_path);
    if (access(resource_path.c_str(), F_OK))
        throw (ResponseHandlerError("HTTP/1.1 409 Conflict", 409));
    if (req.getPath()[req.getPath().size() - 1] == '/')
        throw (ResponseHandlerError("HTTP/1.1 403 Forbidden", 403));
    SetResponseHeader(req, "HTTP/1.1 200 OK", -1);
    target_file->open(resource_path.c_str(), std::ios::out);
    if (!target_file->is_open())
        throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
    is_post = true;
}

void ResponseHandler::ProccessHttpDELETE(HttpRequest &req)
{
    if (!access(resource_path.c_str(), R_OK) || IsDir(resource_path.c_str()))
        throw("HTTP/1.1 403 Forbidden");
    if (std::remove(resource_path.c_str()) == -1)
        throw("HTTP/1.1 500 Internal Server Error");
    SetResponseHeader(req, "HTTP/1.1 200 OK", -1);
}

ResponseHandler::~ResponseHandler(){
    delete target_file;
}
