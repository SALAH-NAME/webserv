#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()
#include <iostream>
#include <string>
#include <string.h>
#include <arpa/inet.h> // for inet_addr()

using namespace std;

int createSocket(void) {
	int i;
	int		port = 8080;
	string  host = "127.0.0.1";
	int		domin = AF_INET;
	int		type = SOCK_STREAM | SOCK_NONBLOCK;
	
	int fd = socket(AF_INET, type, 0);
	std::cout << "fd => " << fd << "\n";
	
	sockaddr_in _Address;
	memset(&_Address, 0, sizeof(_Address));
	_Address.sin_family = domin;
	_Address.sin_port = htons(port);
	i = inet_pton(AF_INET, host.c_str(), &_Address.sin_addr);
	cout << "inet_pton ==> " << i << "\n";
	i = bind(fd, (sockaddr*)&_Address, sizeof(_Address));
	std::cout << "bind ==> " << i << "\n";

	if (errno == EADDRINUSE)
		cout << "IP already used\n";

	fd = listen(fd, 10);
	std::cout << "listen ==> " << listen << "\n";

	return fd;
}

int main() {

	int fd1 = createSocket();
	// cout << "-----------------------------------\n";
	// int fd2 = createSocket();
	// close(fd1);

	cout << "-----------------------------------\n";
	cout << "errno ==> " << errno << "\n";
	while (true) {}
}