#include <map>
#include <iostream>
#include <sstream>

bool locationMatched(std::string req_path, std::string loc_path)
{
    //first part will only compare the request path and the location path
    std::stringstream req_path_ss(req_path.c_str());
    std::stringstream loc_path_ss(loc_path.c_str());
    std::string req_part;
    std::string loc_part;

    while (std::getline(loc_path_ss, loc_part, '/'))
    {
        std::getline(req_path_ss, req_part, '/');
        if (loc_part != req_part)
            return false;
    }
    std::getline(req_path_ss, req_part);
    std::cout << req_part<<std::endl; 
    return true;
}

int main()
{
    std::cout << locationMatched("/media/test/images/img.jpg", "/media/test");
}
