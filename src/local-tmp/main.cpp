#include "ResponseHandler.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "Request.hpp"
#include "test-cases/includeme.hpp"

#define TESTS_NB 5

int main(int ac, char **av)
{
	int chosen_test;
	if (ac != 2)
		return 1; //to-do: use all tests instead
	chosen_test = std::atoi(av[1]);
	if (chosen_test < 1 || chosen_test > TESTS_NB)
		return 1;
	std::string (*test[])() = {test1, test2, test3, test4, test5};
	std::cout << test[std::atoi(av[1]) - 1]();
}
