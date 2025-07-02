#ifndef FILE_HPP
#define FILE_HPP

#include <cctype>
#include <cstddef>
#include <string>
#include <fcntl.h>
#include <unistd.h>

class File
{
private:
	int _fd;
	bool _ownsFd;
	std::string _path;

	/*friend class Pipe;*/

public:
	File();
	explicit File(int fd);
	File(const File& other);
	File& operator=(const File& other);
	File(const char* path, int flags, mode_t mode = 0644);
	~File();
	operator int() const;

	void open(const char* path, int flags, mode_t mode = 0644);
	ssize_t read(void* buf, size_t count);
	ssize_t write(const void* buf, size_t count);
	int getFd() const;
	bool isOpen() const;
	void close();
	File duplicate() const;
	void duplicateTo(int targetFd);
	void stat(struct stat* st);
	bool access(int mode) const;
	const std::string& getPath() const;
};

#endif // !FILE_HPP
