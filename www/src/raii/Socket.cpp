#include "Socket.hpp"
#include <asm-generic/socket.h>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

Socket::Socket() : _fd(-1), _ownsFd(false) {};

Socket::Socket(int fd) : _fd(fd), _ownsFd(true)
{
	if (_fd < 0)
		throw std::runtime_error("Invalid file descriptor");
}

Socket::Socket(const Socket& other) : _fd(other._fd), _ownsFd(other._ownsFd)
{
    const_cast<Socket&>(other)._fd = -1;
    const_cast<Socket&>(other)._ownsFd = false;
}

Socket& Socket::operator=(const Socket& other)
{
    if (this != &other) {
        close();
        _fd = other._fd;
        _ownsFd = other._ownsFd;
        const_cast<Socket&>(other)._fd = -1;
        const_cast<Socket&>(other)._ownsFd = false;
    }
    return *this;
}

Socket::~Socket()
{
	close();
}

Socket::operator int() const
{
	return _fd;
}

void Socket::create(int domain, int type, int protocol)
{
	close();
	_fd = socket(domain, type, protocol);
	if (_fd < 0)
		throw std::runtime_error(std::string("Failed to create socket: ") + strerror(errno));
	_ownsFd = true;
}

void Socket::bind(const struct sockaddr* addr, socklen_t addrlen)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	if (::bind(_fd, addr, addrlen) < 0)
		throw std::runtime_error(std::string("Failed to bind socket: ") + strerror(errno));
}

void Socket::listen(int backlog)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	if (::listen(_fd, backlog) < 0)
		throw std::runtime_error(std::string("Failed to listen on socket: ") + strerror(errno));
}

Socket Socket::accept(struct sockaddr* addr, socklen_t* addrlen)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	int clientFd = ::accept(_fd, addr, addrlen);
	if (clientFd < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			throw "No more pending connections";
		}
		throw std::runtime_error(std::string("Failed to accept connection: ") + strerror(errno));
	}
	return Socket(clientFd);
}

void Socket::connect(const struct sockaddr* addr, socklen_t addrlen)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	if (::connect(_fd, addr, addrlen) < 0)
		throw std::runtime_error(std::string("Failed to connect: ") + strerror(errno));
}

ssize_t Socket::send(const void* buf, size_t len, int flags)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	ssize_t result = ::send(_fd, buf, len, flags);
	if (result < 0)
		throw std::runtime_error(std::string("Failed to send data: ") + strerror(errno));
	return result;
}

ssize_t Socket::recv(void* buf, size_t len, int flags)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	return ::recv(_fd, buf, len, flags);
}

void Socket::setsockopt(int level, int optname, const void* optval, socklen_t optlen)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	if (::setsockopt(_fd, level, optname, optval, optlen) < 0)
		throw std::runtime_error(std::string("Failed to get socket name: ") + strerror(errno));
}

void Socket::getsockname(struct sockaddr* addr, socklen_t* addrlen)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	if (::getsockname(_fd, addr, addrlen) < 0)
		throw std::runtime_error(std::string("Failed to get socket name: ") + strerror(errno));
}

void Socket::setReuseaddr(bool reuse)
{
	int opt = reuse ? 1 : 0;
	setsockopt(SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

int Socket::getFd() const
{
	return _fd;
}

bool Socket::isValid() const
{
	return _fd >= 0;
}

int Socket::release()
{
	int tempFd = _fd;
	_fd = -1;
	_ownsFd = false;
	return tempFd;
}

void Socket::close()
{
	if (_fd >= 0 && _ownsFd)
		::close(_fd);
	_fd = -1;
	_ownsFd = false;
}

Socket Socket::duplicate() const
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	int newFd = dup(_fd);
	if (newFd < 0)
		throw std::runtime_error(std::string("Failed to duplicate socket: ") + strerror(errno));
	return Socket(newFd);
}

void Socket::duplicateTo(int targetFd)
{
	if (_fd < 0)
		throw std::runtime_error("Socket not initialized");
	if (dup2(_fd, targetFd) < 0)
		throw std::runtime_error(std::string("Failed to duplicate socket to target fd: ") + strerror(errno));
}
