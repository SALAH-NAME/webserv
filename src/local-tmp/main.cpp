#include "ResponseHandler.hpp"
#include "ConfigManager.hpp"
#include "ConfigPrinter.hpp"
#include "test-cases/includeme.hpp"
#include <ostream>

#define TESTS_NB 5

bool quick_pars(int &chosen_test, char **av, int ac)
{
	if (ac == 1 || ac > 3){
		std::cerr << "usage: ./test <optional -c flag> <test-number>\n";
		std::exit(1);
	}
	if (std::string(av[1]) == "-c")
	{
		if (ac == 3)
			chosen_test = std::atoi(av[2]);
		if (chosen_test < 0 || chosen_test > TESTS_NB){
			std::cerr << "test number must be in range [1 - " << TESTS_NB << "]\n";
			std::exit(1);
		}
		return true;
	}
	if (ac == 3){
		std::cerr << "first arg can only be '-c'\n";
		std::exit(1);
	}
	else
		chosen_test = std::atoi(av[1]);
	if (chosen_test <= 0 || chosen_test > TESTS_NB){
		std::cerr << "test number must be in range [1 - " << TESTS_NB << "]\n";
		std::exit(1);
	}
	return false;
}

#include <fstream>

void compairOutputs(std::string current_output, int test_nb)
{
	std::cout << "------------------------------------------------\n";
	std::string ref_path = "test-cases/expected-outputs/test" + NumtoString(test_nb) + ".txt";
	std::cout << "comparing test nb: " << test_nb << " with path:\n" << ref_path << std::endl;
	std::ifstream expected_file(ref_path.c_str());
	if (!expected_file.is_open())
		std::cerr << "Failed to open expected output file.\n";
	std::istringstream current_stream(current_output);
	std::string expected_line, current_line;
	int line_num = 1;
	bool success = true;

	while (std::getline(expected_file, expected_line) && std::getline(current_stream, current_line)) {
		if (expected_line != current_line) {
			std::cout << "Line " << line_num << " differs:\n";
			std::cout << "Expected: " << expected_line << "\n";
			std::cout << "Actual  : " << current_line << "\n";
			success = false;
		}
		++line_num;
	}

	// Check for extra lines in either output
	while (std::getline(expected_file, expected_line)) {
		std::cout << "Line " << line_num << " missing in actual output:\n";
		std::cout << "Expected: " << expected_line << "\n";
		success = false;
		++line_num;
	}
	while (std::getline(current_stream, current_line)) {
		std::cout << "Line " << line_num << " extra in actual output:\n";
		std::cout << "Actual  : " << current_line << "\n";
		success = false;
		++line_num;
	}

	if (success)
		std::cout << "SUCCESS!" << std::endl;
	else
		std::cout << "FAILED!" << std::endl;
	std::cout << "------------------------------------------------\n\n\n";
}


int main(int ac, char **av)
{
	int chosen_test = 0;
	bool compair_output = quick_pars(chosen_test, av, ac);
	std::string (*tests[])() = {test1, test2, test3, test4, test5};
	if (!compair_output){
		std::cout << tests[chosen_test-1]();
		return 0;
	}
	else if (chosen_test != 0)
		return (compairOutputs(tests[chosen_test-1](), chosen_test), 0);
	while (chosen_test < TESTS_NB)
	{
		compairOutputs(tests[chosen_test](), chosen_test+1);
		chosen_test++;
	}
}
