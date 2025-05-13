#include "Environment.hpp"
#include <string.h>
#include <stdio.h>

using namespace std;
	
int main(int ac, char **av, char **envp)
{
	Environment test(envp);
	test.display();
}
