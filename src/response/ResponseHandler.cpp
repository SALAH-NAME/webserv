#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler()
{
    InitializeStandardContentTypes();
    InitializeStatusPhrases();
    req = NULL;
    conf = NULL;
    client_info.clientAddr ="";
    client_info.port = "0";
    loc_config = NULL;
    is_location_allocated = false;
    resource_path = "";
    require_cgi = false;
    is_post = false;
    keep_alive = false;
    target_file = NULL;
    cgi_tmpfile_id = -1;
    cgi_buffer_size = 0;
}

void    ResponseHandler::CheckForContentType()
{
    try{
        CgiObj.PreBodyPhraseChecks();
    }
    catch (CgiHandler::BadCgiOutput &ex){
        throw (ResponseHandlerError(req->getVersion() + " 502 Bad Gateway", 500));
    }
}

void ResponseHandler::SetServerConf(ServerConfig *usedConf, const ClientInfos &infos){
    conf = usedConf;
    client_info.clientAddr = infos.clientAddr;
    client_info.port = infos.port;
}

std::string	ResponseHandler::GetResponseHeader(){return response_header;}

std::string ResponseHandler::GetResponseBody(){return response_body;}

std::fstream *ResponseHandler::GetTargetFilePtr(){return target_file;}

bool ResponseHandler::IsPost(){return (is_post || CgiObj.PostReq());}

std::string ResponseHandler::GetResourcePath() {return resource_path;}

bool ResponseHandler::KeepConnectioAlive() {return keep_alive;}

void ResponseHandler::RefreshData()
{
    DeleteCgiTargetFile();
    response_header.clear();
    response_body.clear();
    resource_path.clear();
    require_cgi = false;
    is_post = false;
    req = NULL;
    client_info.clientAddr = "";
    client_info.port = "0";
    cgi_buffer_size = 0;
    child_status = -1;
    cgi_tmpfile_id = -1;
    if (is_location_allocated){
        delete loc_config;
        loc_config = NULL;
        is_location_allocated = false;
    }
    loc_config = NULL;
    if (target_file){
        target_file->close();
        delete target_file;
        target_file = NULL;
    }
//     std::cout << "===========> REFRESHED DATA <===========" << std::endl;//logger
}

void ResponseHandler::Run(HttpRequest &request)
{
//     std::cout << "RESSPONSE RUN \n";//logger
//     std::cout << "=========> passed request to run <========\n" << "method: "//logger
//         << request.getMethod() << std::endl << "path: " << request.getPath() << std::endl;//logger
    RefreshData();
    if (conf == NULL)
        LoadErrorPage(req->getVersion() + " 500 Internal Server Error", 500);
    keep_alive = false;
    req = &request;
    try {
        ProccessRequest();
    }
    catch (ResponseHandlerError &my_exception){
        LoadErrorPage(my_exception.what(), my_exception.getStatusCode());
    }
    catch (std::bad_alloc &insuficcent_mem){
        LoadErrorPage(req->getVersion() + " 503 Service Unavailable", 503);
    }
    catch (std::exception &ex){
        LoadErrorPage(req->getVersion() + " 500 Internal Server Error", 500);
    }
    
}

void ResponseHandler::SetKeepAlive()
{
    if (req->getVersion() == "HTTP/1.0" || req->getHeaders().find("connection") == req->getHeaders().end())
        keep_alive = false;
    else
        keep_alive = req->getHeaders()["connection"] == "keep-alive" ? true : false;
}

void ResponseHandler::ProccessRequest()
{
//     std::cout << "inside process request" << std::endl; //logger
    InitialRequestCheck();
//     std::cout << "passed initial error checks" << std::endl; //logger
    SetKeepAlive();
    RouteResolver(req->getPath(), req->getMethod());//  set resource_path and loc_config
    // std::cout << "route resoulved to " << resource_path << std::endl;
    try {
        req->validateContentLengthLimit(loc_config->getClientMaxBodySize());
    }
    catch (const HttpRequestException &e)
    {
        throw (ResponseHandlerError(req->getVersion() + " " + NumtoString(e.statusCode()) + " " + e.what(), e.statusCode()));
    }
//     std::cout << "content length validated" << std::endl;//logger
    if (NeedToRedirect())
        return (GenerateRedirection());
//     std::cout << "no need for redirection" << std::endl;//logger
    if (loc_config->getAllowedMethods().find(stringToHttpMethod(req->getMethod())) == loc_config->getAllowedMethods().end())
        throw (ResponseHandlerError(req->getVersion() + " 405 Not Allowed", 405));//req method is not allowed on the route
    switch (stringToHttpMethod(req->getMethod()))
    {
        case HTTP_GET:
            ProccessHttpGET();
            break;
        case HTTP_POST:
            ProccessHttpPOST();
            break;
        case HTTP_DELETE:
            ProccessHttpDELETE();
            break;
    }
}   

void    ResponseHandler::HandleDirRequest()
{
    const std::vector<std::string> &indexes = loc_config->getIndex();
    std::string current_path;
    if (indexes.empty())
        return (GenerateDirListing());
    for (unsigned int i=0;i<indexes.size();i++)
    {
        current_path = resource_path + '/' + indexes[i];
        if (access(current_path.c_str(), R_OK) == 0)
            return (LoadStaticFile(current_path));
    }
    if (loc_config->getAutoindex())
        return (GenerateDirListing());
    else 
        throw (ResponseHandlerError(req->getVersion() + " 403 Forbidden", 403));
}

void ResponseHandler::ProccessHttpGET()
{
//     std::cout << "inside GET req processor" << std::endl;//logger
    if (require_cgi)
        return (CgiObj.RunCgi(*req, *conf, *loc_config, resource_path, client_info));    
    if (access(resource_path.c_str(), R_OK) != 0 || (IsDir(resource_path.c_str())
        && loc_config->getIndex().empty() && !loc_config->getAutoindex()))
            throw (ResponseHandlerError(req->getVersion() + " 403 Forbidden", 403));
    if (IsDir(resource_path.c_str()))
        return (HandleDirRequest());
    return (LoadStaticFile(resource_path));
}

void ResponseHandler::ProccessHttpPOST()
{
//     std::cout << "inside post req processor" << std::endl;//logger
    if (require_cgi)
        return (CgiObj.RunCgi(*req, *conf, *loc_config, resource_path, client_info));
    if (access(resource_path.c_str(), F_OK) == 0){
        keep_alive = false;
        throw (ResponseHandlerError(req->getVersion() + " 409 Conflict", 409));
    }
    if (access(GetFileDirectoryPath(resource_path).c_str(), W_OK | X_OK) != 0 ||
            req->getPath()[req->getPath().size() - 1] == '/'){
        keep_alive = false;
        throw (ResponseHandlerError(req->getVersion() + " 403 Forbidden", 403));}
    SetResponseHeader(req->getVersion() + " 201 Created", -1, false);
    target_file = new std::fstream(resource_path.c_str(), std::ios::out);
    if (!target_file || !target_file->is_open()){
        keep_alive = false;
        throw (ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));
    }
    is_post = true;
}

void ResponseHandler::ProccessHttpDELETE()
{
    if (access(resource_path.c_str(), R_OK) != 0 || IsDir(resource_path.c_str()))
        throw(ResponseHandlerError(req->getVersion() + " 403 Forbidden", 403));
    if (std::remove(resource_path.c_str()) == -1)
        throw(ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));
    SetResponseHeader(req->getVersion() + " 200 OK", 0, false);
}

ResponseHandler::~ResponseHandler(){
	DeleteCgiTargetFile();
    if (is_location_allocated){
        delete loc_config;
    }
    if (target_file) {
		target_file->close();
		delete target_file;
		target_file = NULL;
	}
}
