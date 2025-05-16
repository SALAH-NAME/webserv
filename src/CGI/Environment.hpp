#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <cstring>

class Environment{
	private:
		std::map<std::string, std::string> env;
	public:
		Environment(char **envp);
		void Add(const std::string key, const std::string value);
		char **GetRawEnv();
		void display();
};

void delete_env(char **env);

#endif