#include "File.hpp"
#include "Pipe.hpp"
#include <cerrno>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

static void printTestHeader(const std::string& testName)
{
	std::cout << "\n=== " << testName << " ===\n";
}

static void printTestResult(const std::string& testCase, bool passed)
{
	std::cout << testCase << ": " << (passed ?  GREEN "PASSED" RESET : RED "FAILED" RESET) << std::endl;
}

static void testPipeBasicOperations()
{
	printTestHeader("Pipe Basic Operations");

	{
		Pipe p;
		bool passed = (!p.isReadOpen() && !p.isWriteOpen() && p.getReadFd() == -1 && p.getWriteFd() == -1);
		printTestResult("Default constructor", passed);
	}

	{
		bool passed = false;
		try
		{
			Pipe p(true);
			passed = (p.isReadOpen() && p.isWriteOpen() && p.getReadFd() >= 0 && p.getWriteFd() >= 0);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Pipe creation", passed);
	}

	{
		bool passed = false;
		try
		{
			Pipe p(true);
			std::string data = "pipe test data";

			ssize_t written = p.write(data.c_str(), data.length());

			char buffer[100] = {0};
			ssize_t bytesRead = p.read(buffer, sizeof(buffer) - 1);
			passed = (written == static_cast<ssize_t>(data.length()) && bytesRead == written && std::string(buffer) == data);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Pipe write/read operations", passed);
	}
}


static void testPipeRAII()
{
	printTestHeader("Pipe RAII Implementation");

	{
		int readFd = -1, writeFd = -1;
		{
			Pipe p(true);
			readFd = p.getReadFd();
			writeFd = p.getWriteFd();
		}
		bool readClosed = (read(readFd, NULL, 0) == -1 && errno == EBADF);
		bool writeClosed = (write(writeFd, "test", 4) == -1 && errno == EBADF);

		printTestResult("Automatic cleanup on destruction", readClosed && writeClosed);
	}

	{
		bool passed = false;
		try
		{
			Pipe p(true);
			p.closeWrite();

			passed = (!p.isWriteOpen() && p.isReadOpen() && p.getWriteFd() == -1);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Selective close (write end)", passed);
	}

	{
		bool passed = false;
		try
		{
			Pipe p(true);
			int originalReadFd = p.getReadFd();
			int releasedFd = p.releaseRead();

			passed = (releasedFd == originalReadFd && !p.isReadOpen() && p.getReadFd() == -1);
			if (releasedFd >= 0)
				close(releasedFd);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("FD release without closing", passed);
	}
}

static void testPipeDuplication()
{
	printTestHeader("Pipe Duplication");
	
	{
		bool passed = false;
		try
		{
			Pipe p(true);
			File readFile = p.duplicateRead();

			std::string data = "dup read test";
			p.write(data.c_str(), data.length());
			p.closeWrite();

			char buffer[100] = {0};
			ssize_t bytesRead = readFile.read(buffer, sizeof(buffer) - 1);

			passed = (bytesRead > 0 && std::string(buffer) == data);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Duplicate read end", passed);
	}

	{
		bool passed = false;
		try
		{
			Pipe p(true);
			File writeFile = p.duplicateWrite();

			std::string data = "dup write test";
			writeFile.write(data.c_str(), data.length());
			writeFile.close();
			p.closeWrite();

			char buffer[100] = {0};
			ssize_t bytesRead = p.read(buffer, sizeof(buffer) - 1);
			passed = (bytesRead > 0 && std::string(buffer) == data);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Duplicate write end", passed);
	}
}

static void testErrorHandling()
{
	bool pipeException = false;
	try
	{
		Pipe p;
		p.read(NULL, 0);
	} catch (const std::exception& e)
	{
		pipeException = true;
	}
	printTestResult("Invalid operations throw exceptions", pipeException);
}

void testPipe()
{
	testPipeBasicOperations();
	testPipeRAII();
	testPipeDuplication();
	testErrorHandling();
}
