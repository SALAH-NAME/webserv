#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <cstddef>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Socket
{
private:
	int _fd;
	bool _ownsFd;

public:
	Socket();
    Socket(int fd);
    Socket(const Socket& other); // ownership-transfer copy
    Socket& operator=(const Socket& other); // ownership-transfer assignment
    ~Socket();
    operator int() const;

	void create(int domain = AF_INET, int type = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, int protocol = 0); // Non-Blocking
	void bind(const struct sockaddr* addr, socklen_t addrlen);
	void listen(int backlog = SOMAXCONN);
	Socket accept(struct sockaddr* addr = NULL, socklen_t* addreln = NULL);
	void connect(const struct sockaddr* addr, socklen_t addrlen);
	ssize_t send(const void* buf, size_t len, int flags = 0);
	ssize_t recv(void* buf, size_t len, int flags = 0);
	void setsockopt(int level, int optname, const void* optval, socklen_t optlen);
	void getsockname(struct sockaddr* addr, socklen_t* addrlen);
	void setReuseaddr(bool reuse = true);
	int getFd() const;
	bool isValid() const;
	int release();
	void close();
	Socket duplicate() const;
	void duplicateTo(int targetFd);
};


#endif // !SOCKET_HPP
