#include "Environment.hpp"

using namespace std;

Environment::Environment(char **envp){}

void Environment::display()
{
	for (map<string,string>::iterator iter = env.begin(); iter != env.end(); iter++)
		cout << iter->first << "=" << iter->second << endl;
}

void Environment::Add(const string key, const string value)
{
	env[key] = value;
}

char **Environment::GetAllEnv()
{
	char **res = new char*[env.size() + 1];
	int i=0;
	res[env.size()] = NULL;
	for(map<string, string>::iterator iter = env.begin(); iter != env.end();iter++)
	{
		res[i] = new char[(iter->first + iter->second).size() + 2];
		std::strcpy(res[i], (iter->first + "=" + iter->second).c_str());
		i++;
	}
	return res;
}

void delete_env(char **env)
{
	for (int i=0;env[i];i++)
		delete[] env[i];
	delete[] env;
}