#include "ResponseHandler.hpp"

void ResponseHandler::InitializeStatusPhrases()
{
    status_phrases[200] = std::string("OK");
    status_phrases[302] = std::string("Found");
    status_phrases[403] = std::string("Forbidden");
    status_phrases[401] = std::string("Unauthorized");
    status_phrases[201] = std::string("Created");
    status_phrases[204] = std::string("No Content");
    status_phrases[301] = std::string("Moved Permanently");
    status_phrases[304] = std::string("Not Modified");
    status_phrases[400] = std::string("Bad Request");
    status_phrases[404] = std::string("Not Found");
    status_phrases[501] = std::string("Not Implemented");
    status_phrases[502] = std::string("Bad Gateway");
    status_phrases[503] = std::string("Service Unavailable");
    status_phrases[504] = std::string("Gateway Timeout")    ;
    status_phrases[413] = std::string("Payload Too Large");
    status_phrases[500] = std::string("Internal Server Error");
    status_phrases[405] = std::string("Method Not Allowed");
}

std::string GetFormattedEntryInfo(std::string name, const std::string &time_stamp, const std::string &size)
{
    std::stringstream ss;

    if (name.size() > 50){
        name.erase(47);
        name+="..&gt;";
        ss << std::left << std::setw(51) << std::setfill(' ') << (name + "</a> ");
    }
    else 
        ss << std::left << std::setw(55) << std::setfill(' ') << (name + "</a>");
    ss << time_stamp;
    ss << std::right <<std::setw(20) << std::setfill(' ') << size;
    return (ss.str());
}

void ResponseHandler::SetResponseHeader(const std::string &status_line, int len,
        bool is_static, std::string location)
{
    response_header = status_line + CRLF + "Server: " + SRV_NAME + CRLF + "Date: " +
        GenerateTimeStamp() + CRLF;
    response_header += "Connection: " + std::string(keep_alive ? "keep-alive" : "close") + CRLF;
    if (len != -1)
    {
        response_header += is_static ? GenerateContentType(resource_path) : "Content-Type: text/html";
        response_header += CRLF + std::string("Content-Length: ") + NumtoString(len) + CRLF;
    }
    if (!location.empty())
        response_header += location + CRLF;
    response_header += CRLF;
}

void    ResponseHandler::GenerateDirListing()
{
    DIR                         *dir;
    std::vector<std::string>    dir_entries;
    struct dirent               *dir_iter;
    struct stat                 file_stat;
    std::string                 element_last_mod_date;
    std::string                 element_size_in_bytes;

    dir = opendir(resource_path.c_str());
    if (!dir)
        throw (ResponseHandlerError(req->getVersion() + " 500 Internal Server Error", 500));
    while ((dir_iter = readdir(dir)) != NULL)
        if (static_cast <std::string>(dir_iter->d_name) != ".")
            dir_entries.push_back(dir_iter->d_name);
    std::sort(dir_entries.begin(), dir_entries.end());
    response_body = "<html>\n\t<head><title>Index of " + req->getPath() + "</title></head>\n\t<body>\n";
    response_body += "\t\t<h1>Index of " + req->getPath() + "</h1><hr>\n\t\t<pre>";
    for (std::vector<std::string>::iterator it = dir_entries.begin(); it != dir_entries.end(); it++)
    {
        if (stat((resource_path + *it).c_str(), &file_stat) == 0)
        {
            element_last_mod_date = formatDate("%d-%b-%Y %R", file_stat.st_atim.tv_sec, 17);
            element_size_in_bytes = NumtoString(file_stat.st_size); 
            response_body += "\n<a href=\"" + *it + "\">";
            response_body += GetFormattedEntryInfo(*it, element_last_mod_date, element_size_in_bytes);
        }
    }
    response_body += "</pre><hr>\n\t</body>\n</html>";
    SetResponseHeader(req->getVersion() + " 200 OK", response_body.size(), false);
    closedir(dir); 
}

void ResponseHandler::GenerateRedirection()
{
    std::string status_code = NumtoString(301);
    std::string location;
    std::string http_message = " Moved Permanently";
    if (IsDir(resource_path.c_str()) && !loc_config->hasRedirect())
        location = "Location: http://" + req->getHeaders()["host"] + req->getPath() + '/';
    else {
        location = "Location: " + loc_config->getRedirect().url;
        status_code = NumtoString(loc_config->getRedirect().status_code);
    }
    if (status_code == "302" || status_code == "307")
        http_message = " Moved Temporary";
    response_body = 
        "<html>\n<head><title>" + status_code + http_message +
        "</title></head>\n<body>\n<center><h1>" + status_code + http_message +
        "</h1></center>\n<hr><center>" + std::string(SRV_NAME) + "</center>\n"
        "</body>\n</html>";
    SetResponseHeader(req->getVersion() + " " + status_code + http_message, response_body.size(), false, location);
}
