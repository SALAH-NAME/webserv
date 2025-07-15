#ifndef PIPE_HPP
#define PIPE_HPP

#include <cstddef>
#include <unistd.h>
#include "File.hpp"

class Pipe
{
private:
	int _readFd;
	int _writeFd;
	int _ownsRead;
	int _ownsWrite;

	// Prevent copying
	Pipe(const Pipe&);
	Pipe& operator=(const Pipe&);

public:
	Pipe();
	explicit Pipe(bool createNow);
	Pipe(int readFd, int writeFd, bool ownsRead = true, bool ownsWrite = true);
	~Pipe();

	void create();
	void closeRead();
	void closeWrite();
	void close();
	ssize_t read(void* buf, size_t count);
	ssize_t write(const void* buf, size_t count);
	int getReadFd() const;
	int getWriteFd() const;
	bool isReadOpen() const;
	bool isWriteOpen() const;
	int releaseRead();
	int releaseWrite();
	File duplicateRead() const;
	File duplicateWrite() const;
};

#endif // !PIPE_HPP
