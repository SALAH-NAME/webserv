#include "ResponseHandler.hpp"

void ResponseHandler::InitializeStandardContentTypes()
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

void	ResponseHandler::LoadStaticFile(Request &req, const std::string &file_path)
{
    struct stat path_info;

	resource_path = file_path;
	if (stat(file_path.c_str(), &path_info) != 0)
		throw (ResponseHandlerError("HTTP/1.1 500 Internal Server Error", 500));
	SetResponseHeader(req, "HTTP/1.1 200 OK", path_info.st_size);
	*target_file = File(file_path.c_str(), O_RDONLY, 0);
}
