#include <string>
#include <iostream>

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
