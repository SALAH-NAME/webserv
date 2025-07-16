#include "File.hpp"
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>


File::File() : _fd(-1), _ownsFd(false), _path() {}

File::File(int fd) : _fd(fd), _ownsFd(true), _path()
{
	if (_fd < 0)
		throw std::runtime_error("Invalid file descriptor");
}

File::File(const char* path, int flags, mode_t mode) : _fd(-1), _ownsFd(false), _path()
{
	open(path, flags, mode);
}

File::File(const File& other) : _fd(-1), _ownsFd(false), _path()
{
	if (other._fd >= 0 && other._ownsFd)
	{
		_fd = dup(other._fd);
		if (_fd < 0)
			throw std::runtime_error(std::string("Failed to duplicate file in copy constructor: ") + strerror(errno));

		_ownsFd = true;
		_path = other._path;
	}
}

File& File::operator=(const File& other)
{
	if (this != &other)
	{
		close();
		if (other._fd >= 0 && other._ownsFd)
		{
			_fd = dup(other._fd);
			if (_fd < 0)
				throw std::runtime_error(std::string("Failed to duplicate file in assignment: ") + strerror(errno));
			_ownsFd = true;
			_path = other._path;
		}
	}
	return *this;
}

File::operator int() const
{
	return _fd;
}

File::~File()
{
	close();
}

void File::open(const char* path, int flags, mode_t mode)
{
	close();
	_fd = ::open(path, flags, mode);
	if (_fd < 0)
		throw std::runtime_error(std::string("Failed to open file '") + path + "': " + strerror(errno));
	_ownsFd = true;
	_path = path;
}

ssize_t File::read(void* buf, size_t count)
{
	if (_fd < 0)
		throw std::runtime_error("File not opened");
	return ::read(_fd, buf, count);
}

ssize_t File::write(const void* buf, size_t count)
{
	if (_fd < 0)
		throw std::runtime_error("File not opened");
	ssize_t result = ::write(_fd, buf, count);
	if (result < 0)
		throw std::runtime_error(std::string("Failed to write to file: ") + strerror(errno));
	return result;
}

int File::getFd() const
{
	return _fd;
}

bool File::isOpen() const
{
	return _fd >= 0;
}

void File::close()
{
	if (_fd >= 0 && _ownsFd)
		::close(_fd);
	_fd = -1;
	_ownsFd = false;
	_path.clear();
}

File File::duplicate() const
{
	if (_fd < 0)
		throw std::runtime_error("Failed not opened");
	int newFd = dup(_fd);
	if (newFd < 0)
		throw std::runtime_error(std::string("Failed to duplicate file: ") + strerror(errno));
	return File(newFd);
}

void File::duplicateTo(int targetFd)
{
	if (_fd < 0)
		throw std::runtime_error("File not opened");
	if (dup2(_fd, targetFd) < 0)
		throw std::runtime_error(std::string("Failed to duplicate file to target fd: ") + strerror(errno));
}

void File::stat(struct stat* st)
{
	if (_fd < 0)
		throw std::runtime_error("File not opened");
	if (::stat(_path.c_str(), st) < 0 )
		throw std::runtime_error(std::string("Failed to get file stats: ") + strerror(errno));
}

bool File::access(int mode) const
{
	if (_path.empty())
		return false;
	return ::access(_path.c_str(), mode) == 0;
}

const std::string& File::getPath() const
{
	return _path;
}
