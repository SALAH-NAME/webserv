#include "ResponseHandler.hpp"

bool IsDir(const char  *path)
{
    struct stat file_stat;
    if (stat(path, &file_stat) != 0)
        return false;
    return (S_ISDIR(file_stat.st_mode));
}

std::string formatDate(const char *format, time_t time, int len)
{
    struct tm* time_info;
    std::string result;
    char *tab = new char[len+1];

    tab[len] = 0;
    time_info = std::localtime(&time);
    strftime (tab,len+1, format, time_info);
    result = tab;
    delete[] tab;
    return (result);
}

std::string GenerateTimeStamp()
{
    time_t current_time;
    std::time(&current_time);

    return (formatDate("%a, %d %b %Y %H:%M:%S GMT", current_time, 29));
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
