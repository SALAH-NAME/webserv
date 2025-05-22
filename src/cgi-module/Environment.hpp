#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <cstring>

class Environment{// a class to deal with the environment
	private:
		std::map<std::string, std::string> env;//using the map to store env vars as key value pairs of strings 
	public:
		Environment();
		void Add(const std::string key, const std::string value);
		char **GetRawEnv();//return the data of the object in a (char **) format to comply with the execve format
		void display();
};

void delete_env(char **env);

#endif