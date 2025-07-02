#include "File.hpp"
#include <cerrno>
#include <exception>
#include <fcntl.h>
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


static void testFileBasicOperations()
{
	printTestHeader("File Basic Operations");

	{
		File f;
		printTestResult("Default constructor", !f.isOpen() && f.getFd() == -1);
	}

	{
		bool passed = false;
		try
		{
			File f("deleteme/test_file.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
			std::string data = "Hello, World!";
			ssize_t written = f.write(data.c_str(), data.length());
			passed = (written == static_cast<ssize_t>(data.length()) && f.isOpen());
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("File creation and writing", passed);
	}

	{
		bool passed = false;
		try
		{
			File f("deleteme/test_file.txt", O_RDONLY);
			char buffer[100] = {0};
			ssize_t bytesRead = f.read(buffer, sizeof(buffer) - 1);
			passed = (bytesRead > 0 && std::string(buffer) == "Hello, World!");
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("File reading", passed);
	}

	{
		bool passed = false;
		try
		{
			File f("deleteme/test_file.txt", O_RDONLY);
			passed = (f.getPath() == "deleteme/test_file.txt" && f.access(R_OK));
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("File path and access", passed);
	}

	{
		bool passed = false;
		try
		{
			File f("deleteme/test_file.txt", O_RDONLY);
			passed = (f == f.getFd() && f >= 0);
		} catch ( const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("File conversion operator", passed);
	}

}

static void testFileRAII()
{
	printTestHeader("File RAII Implementation");

	{
		int fdBefore = -1;
		{
			File f("deleteme/test_raii.txt", O_CREAT | O_CREAT, 0644);
			fdBefore = f.getFd();
		}

		bool closed = (write(fdBefore, "test", 4) == -1 && errno == EBADF);
		printTestResult("Automatic cleanup on destruction", closed);
	}

	{
		File f("deleteme/test_raii.txt", O_WRONLY);
		int fd = f.getFd();
		f.close();
		bool passed = (!f.isOpen() && f.getFd() == -1 && fd >= 0);
		printTestResult("Manual close", passed);
	}

	{
		int fd = open("deleteme/test_raii.txt", O_RDONLY);
		bool passed = false;
		if (fd >= 0)
		{
			try
			{
				File f(fd);
				passed = (f.getFd() == fd && f.isOpen());
			} catch (const std::exception& e)
			{
				std::cout << "Exception: " << e.what() << std::endl;
			}
		}
		printTestResult("FD constructor with ownership", passed);
	}
}

static void testFileDuplication()
{
	printTestHeader("File Duplication");

	{
		bool passed = false;
		try
		{
			File f1("deleteme/test_dup.txt", O_CREAT | O_WRONLY, 0644);
			File f2 = f1.duplicate();

			std::string data = "duplicate test";
			f1.write(data.c_str(), data.length());

			passed = (f2.isOpen() && f2.getFd() != f1.getFd() && f2.getFd() >= 0);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("File duplication", passed);
	}


	{
		bool passed = false;
		try
		{
			File f1("deleteme/test_dup.txt", O_WRONLY);
			int originalFd = f1.getFd();

			int tempFd = open("/dev/null", O_WRONLY);
			if (tempFd >= 0)
			{
				f1.duplicateTo(tempFd);

				std::string data = "dup2 test";
				ssize_t written = write(tempFd, data.c_str(), data.length());
				passed = (written > 0 && f1.getFd() == originalFd);
				close(tempFd);
			}
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Duplicate to specific FD", passed);
	}
}

void testFileErrorHandling()
{
	printTestHeader("File Error Handling");
	{
		bool caught = false;
		try
		{
				File f("/invalid/path/file.txt", O_RDONLY);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
			caught = true;
		}
		printTestResult("Invalid file path exception", caught);
	}

	{
		bool caught = false;
		try
		{
			File f(-1);
		} catch (const std::exception& e)
		{
			caught = true;
		}
		printTestResult("Invalid FD constructor exception", caught);
	}
	
	{
		bool caught = false;
		try
		{
			File f;
			char buffer[10];
			f.read(buffer, sizeof(buffer));
		} catch (const std::exception& e)
		{
			caught = true;
		}
		printTestResult("Operations on closed file exception", caught);
	}
}

void testFile()
{
	testFileBasicOperations();
	testFileRAII();
	testFileDuplication();
	testFileErrorHandling();
	/*unlink("deleteme/test_file.txt"), unlink("deleteme/test_raii.txt"), unlink("deleteme/test_dup.txt");*/
}
