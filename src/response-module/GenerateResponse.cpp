#include "ResponseHandler.hpp"

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

std::string ResponseHandler::GenerateContentType(std::string extension)
{
    std::string default_type = "application/octet-stream";
    if (extension.empty())
        return (default_type);
    extension = (extension == ".ico") ? ".vnd.microsoft.icon" : extension;
    extension = (extension == ".js" || extension == ".mjs") ? ".javascript" : extension;
    extension = (extension == ".mp3") ? ".mpeg" : extension;
    for (std::map<std::string, std::vector<std::string> >::iterator it =
            content_types.begin();it != content_types.end();it++)
    {
        for (std::vector<std::string>::iterator innerIt = it->second.begin();
            innerIt != it->second.end(); innerIt++)
            if (*innerIt == extension.c_str()+1)
                return (it->first + *innerIt);
    }
    return default_type;
}

void ResponseHandler::SetResponseHeader(Request &req, const std::string &status_line, int len,
        std::string location = "")
{
    struct stat path_info;

    if (stat(resource_path.c_str(), &path_info) != 0)//if it failed -> server error because file does exist with the right permissions
        throw (RequestError("HTTP/1.1 500 Internal Server Error"));
    len = (len == -1) ? path_info.st_size : len;
    response_header = status_line + CRLF + "server: " + SRV_NAME + CRLF + "Date: " +
        GenerateTimeStamp() + CRLF ;
    response_header += (req.getMethod() == "GET") ? GenerateContentType(ExtractFileExtension(resource_path))
        + CRLF + "Content-Length: " + NumtoString(len) : "";
    response_header += location + std::string(CRLF) + std::string(CRLF);
}

void    ResponseHandler::GenerateDirListing(Request &req)
{
    DIR                         *dir;
    std::vector<std::string>    dir_entries;
    struct dirent               *dir_iter;
    struct stat                 file_stat;
    std::string                 element_last_mod_date;
    std::string                 element_size_in_bytes;

    dir = opendir(resource_path.c_str());
    if (!dir)
        throw (RequestError("HTTP/1.1 Internal Server Error"));
    while ((dir_iter = readdir(dir)) != NULL)
        if (static_cast <std::string>(dir_iter->d_name) != ".")
            dir_entries.push_back(dir_iter->d_name);
    std::sort(dir_entries.begin(), dir_entries.end());
    response_body = "<html>\n\t<head><title>Index of /" + req.getPath() + "/</title></head>\n\t<body>\n";
    response_body += "\t\t<h1>Index of /" + req.getPath() + "/</h1><hr>\n\t\t<pre>";
    for (std::vector<std::string>::iterator it = dir_entries.begin(); it != dir_entries.end(); it++)
    {
        if (stat((resource_path + "/" + *it).c_str(), &file_stat))
        {
            element_last_mod_date = formatDate("%d-%b-%Y %R", file_stat.st_atim.tv_sec, 17);
            element_size_in_bytes = NumtoString(file_stat.st_size); 
            response_body += "\n<a href=\"" + *it + "\">";
            response_body + GetFormattedEntryInfo(*it, element_last_mod_date, element_size_in_bytes);
        }
    }
    response_body += "</pre><hr>\n\t</body>\n</html>";
    SetResponseHeader(req, "HTTP/1.1 200 OK", response_body.size());
}

void ResponseHandler::GenerateRedirection(Request &req)
{
    std::string status_code;
    std::string location = NumtoString(301);

    if (IsDir(resource_path.c_str()))
        location = "Location: http://" + req.getHeaders()["Host"] + '/' + req.getPath() + '/';
    else {
        location = "Location: " + loc_config->getRedirect().url;
        status_code = NumtoString(loc_config->getRedirect().status_code);
    }
    response_body = 
        "<html>\n<head><title>301 Moved Permanently"
        "</title></head>\n<body>\n<center><h1>301 Moved Permanently"
        "</h1></center>\n<hr><center>" + std::string(SRV_NAME) + " (Ubuntu)</center>\n"
        "</body>\n</html>";
    SetResponseHeader(req, "HTTP/1.1 " + status_code + " Moved Permanently", response_body.size(), location);
}
