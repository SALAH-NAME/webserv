#include "ResponseHandler.hpp"

bool is_dir(const char  *path)
{
    struct stat file_stat;
    if (stat(path, &file_stat) != 0)
        return false;
    return (S_ISDIR(file_stat.st_mode));
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
            return ("." + result);
    }
}
