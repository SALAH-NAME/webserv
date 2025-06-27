#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include <map>

std::string GenerateContentType(std::string extension, std::map<std::string, std::vector<std::string> >&content_types)
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

std::string ExtractFileExtension(const std::string &path)
{
    std::string result;

    for (int i=path.size()-1; i>=0;i--)
    {
        if (path[i] == '/')
            return "";
        result = path[i] + result;
        if (path[i] == '.')
            return (result);
    }
    return "";
}

int main(int ac , char **av)
{
    std::map<std::string, std::vector<std::string> >content_types;

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
    
    std::string fileName=av[1];
    std::cout << ExtractFileExtension(fileName)<<'\n';
    std::cout<< GenerateContentType(ExtractFileExtension(fileName), content_types) << std::endl;

}