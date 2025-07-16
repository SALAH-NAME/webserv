#include "Pipe.hpp"
#include "File.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <errno.h>
#include <unistd.h>

Pipe::Pipe() : _readFd(-1), _writeFd(-1), _ownsRead(false), _ownsWrite(false) {}

Pipe::Pipe(bool createNow) : _readFd(-1), _writeFd(-1), _ownsRead(false), _ownsWrite(false)
{
	if (createNow)
		create();
}

Pipe::Pipe(int readFd, int writeFd, bool ownsRead, bool ownsWrite)
	: _readFd(readFd), _writeFd(writeFd), _ownsRead(ownsRead), _ownsWrite(ownsWrite)
{
	if ((_readFd < 0 && _ownsRead) || (_writeFd < 0 && _ownsWrite))
		throw std::runtime_error("Invalid file descriptor for pipe");
}

Pipe::~Pipe()
{
	close();
}

void Pipe::create()
{
	close();
	int pipefd[2];
	if (pipe(pipefd) < 0)
		throw std::runtime_error(std::string("Failed to create pipe: ") + strerror(errno));
	_readFd = pipefd[0];
	_writeFd = pipefd[1];
	_ownsRead = true;
	_ownsWrite = true;
}

void Pipe::closeRead()
{
	if (_readFd >= 0 && _ownsRead)
		::close(_readFd);
	_readFd = -1;
	_ownsRead = false;
}

void Pipe::closeWrite()
{
	if (_writeFd >= 0 && _ownsWrite)
		::close(_writeFd);
	_writeFd = -1;
	_ownsWrite = false;
}

void Pipe::close()
{
	closeRead();
	closeWrite();
}

ssize_t Pipe::read(void* buf, size_t count)
{
	if (_readFd < 0)
		throw std::runtime_error("Pipe read end not open");
	return ::read(_readFd, buf, count);
}

ssize_t Pipe::write(const void* buf, size_t count)
{
	if (_writeFd < 0)
		throw std::runtime_error("Pipe write end not open");
	ssize_t result = ::write(_writeFd, buf, count);
	if (result < 0)
		throw std::runtime_error(std::string("Failed to write to pipe: ") + strerror(errno));
	return result;
}

int Pipe::getReadFd() const
{
	return _readFd;
}

int Pipe::getWriteFd() const
{
	return _writeFd;
}

bool Pipe::isReadOpen() const
{
	return _readFd >= 0;
}

bool Pipe::isWriteOpen() const
{
	return _writeFd >= 0;
}

int Pipe::releaseRead()
{
	int tempFd = _readFd;
	_readFd = -1;
	_ownsRead = false;
	return tempFd;
}

int Pipe::releaseWrite()
{
	int tempFd = _writeFd;
	_writeFd = -1;
	_ownsWrite = false;
	return tempFd;
}


File Pipe::duplicateRead() const
{
	if (_readFd < 0)
		throw std::runtime_error("Pipe read end not open");
	int newFd = dup(_readFd);
	if (newFd < 0)
		throw std::runtime_error(std::string("Failed to duplicate pipe read end: ") + strerror(errno));
	return File(newFd);
}

File Pipe::duplicateWrite() const
{
	if (_writeFd < 0)
		throw std::runtime_error("Pipe write end not open");
	int newFd = dup(_writeFd);
	if (newFd < 0)
		throw std::runtime_error(std::string("Failed to duplicate pipe write end: ") + strerror(errno));
	return File(newFd);
}
