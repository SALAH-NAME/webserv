#include "ResponseHandler.hpp"

bool is_dir(const char  *path)
{
    struct stat file_stat;
    if (stat(path, &file_stat) != 0)
        return false;
    return (S_ISDIR(file_stat.st_mode));
}

