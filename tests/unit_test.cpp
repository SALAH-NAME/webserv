#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include "SharedPtr.hpp"
#include "HttpRequest.hpp"

volatile sig_atomic_t g_shutdown = 0;

void testAllTokenizer();
void testConfigPrint();
void testSharedPtr();
void testSocket();
void testFile();
void testPipe();
void testHttpRequestParse();

typedef void (*TestFunction)();

struct Test
{
	std::string	 name;
	TestFunction function;
};

void addTest(std::vector<Test>& tests, const std::string& name, TestFunction function)
{
	Test newTest;
	newTest.name		 = name;
	newTest.function = function;
	tests.push_back(newTest);
}

int displayMenu(const std::vector<Test>& tests)
{
	std::cout << "Select a test to run:" << std::endl;
	for (size_t i = 0; i < tests.size(); ++i)
		std::cout << i + 1 << ". " << tests[i].name << std::endl;

	std::cout << (tests.size() + 1) << ". Run all tests" << std::endl;
	std::cout << "0. Exit" << std::endl;

	int choice;
	std::cout << "Enter your choice: ";
	std::cin >> choice;
	return choice;
}

std::vector<Test> buildTests()
{
	std::vector<Test> tests;

	addTest(tests, "testAllTokenizer", testAllTokenizer);
	addTest(tests, "testConfigPrint", testConfigPrint);
	addTest(tests, "testSharedPtr", testSharedPtr);
	addTest(tests, "testSocket", testSocket);
	addTest(tests, "testFile", testFile);
	addTest(tests, "testPipe", testPipe);
	addTest(tests, "testHttpRequestParse", testHttpRequestParse);

	return tests;
}

int main()
{
	std::vector<Test> tests = buildTests();

	if (true)
	{
		int choice = displayMenu(tests);

		if (choice == 0)
			std::cout << "Exiting..." << std::endl;
		else if (choice > 0 && choice <= static_cast<int>(tests.size()))
			tests[choice - 1].function();
		else if (choice == static_cast<int>(tests.size() + 1))
			for (size_t i = 0; i < tests.size(); ++i)
				tests[i].function();
		else
			std::cout << "Invalid choice. Please try again." << std::endl;
	}

	return 0;
}
