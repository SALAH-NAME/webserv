#include "Socket.hpp"
#include <cerrno>
#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
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

static void testSocketBasicOperations()
{
	printTestHeader("Socket Basic Operations");
	{
		Socket s;
		bool passed = (!s.isValid() && s.getFd() == -1);
		printTestResult("Default constructor", passed);
	}

	{
		bool passed = false;
		try
		{
			Socket s;
			s.create(AF_INET, SOCK_STREAM, 0);
			passed = (s.isValid() && s.getFd() >= 0);
		} catch ( const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Socket creation", passed);
	}

	{
		bool passed = false;
		try
		{
			Socket s;
			s.create();
			s.setReuseaddr(true);
			passed = true;
		}
		catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Socket options (SO_REUSEADDR)", passed);
	}

	{
		bool passed = false;
		try
		{
			Socket s;
			s.create(AF_INET, SOCK_STREAM, 0);
			passed = (s == s.getFd());
		} catch ( const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Socket conversion operator", passed);
	}
}


static void testSocketRAII()
{
	printTestHeader("Socket RAII Implementation");

	{
		int fd = -1;
		{
			Socket s;
			s.create();
			fd = s.getFd();
		}

		bool closed = (send(fd, "test", 4, 0) == -1 && errno == EBADF);
		printTestResult("Automatic cleanup on destruction", closed);
	}

	{
		bool passed = false;
		try
		{
			Socket s;
			s.create();
			s.close();
			passed = (!s.isValid() && s.getFd() == -1);
		}
		catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Manual close", passed);
	}

	{
		bool passed = false;
		try
		{
			Socket s;
			s.create();
			int originalFd = s.getFd();
			int releaseFd = s.release();
			passed = (releaseFd == originalFd && !s.isValid() && s.getFd() == -1);
			if (releaseFd >= 0)
				close(releaseFd);
		} catch (const std::exception& e)
		{
			std::cout << "Execption: " << e.what() << std::endl;
		}
		printTestResult("FD release without closing", passed);
	}
}

static void testSocketServerClient()
{
	printTestHeader("Socket Server/Client Communication");
	
	{
		bool passed = true;
		try
		{
			int sockets[2];
			if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0)
			{
				Socket server(sockets[0]);
				Socket client(sockets[1]);
				
				std::string data = "Hello Server!";
				ssize_t sent = client.send(data.c_str(), data.length(), 0);

				char buffer[100] = {0};
				ssize_t received = server.recv(buffer, sizeof(buffer) - 1, 0);

				passed = (sent > 0 && received > 0 && std::string(buffer) == data);
			} 
		} catch (const std::exception& e)
		{
			std::cout << "Execption: " << e.what() << std::endl;
		}
		printTestResult("Socket communication via socketpair", passed);
	}
}

static void testSocketDuplication()
{
	printTestHeader("Socket Duplication");

	{
		bool passed = false;
		try
		{
			Socket original;
			original.create();
			Socket duplicate = original.duplicate();

			passed = (duplicate.isValid() && duplicate.getFd() != original.getFd() && duplicate.getFd() >= 0);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Socket duplication", passed);
	}

	{
		bool passed = false;
		try
		{
			Socket s;
			s.create();

			int tempSocket = socket(AF_INET, SOCK_STREAM, 0);
			if (tempSocket >= 0)
			{
				s.duplicateTo(tempSocket);

				struct sockaddr_in addr;
				socklen_t addrlen = sizeof(addr);
				int result = getsockname(tempSocket, (struct sockaddr*)&addr, &addrlen);

				passed = (result == 0);
				close (tempSocket);
			}
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
		}
		printTestResult("Duplicate to specific FD", passed);
	}
}

static void testErrorHandling()
{
	printTestHeader("Error Handling Tests");

	{
		bool exception = false;
		try
		{
			Socket s;
			s.send("test", 4, 0);
		} catch (const std::exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
			exception = true;
		}
		printTestResult("Invalid operations throw exceptions", exception);
	}
}

void testSocket()
{
	testSocketBasicOperations();
	testSocketRAII();
	testSocketServerClient();
	testSocketDuplication();
	testErrorHandling();
}
