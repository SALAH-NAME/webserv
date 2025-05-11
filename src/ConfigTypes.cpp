#include "ConfigTypes.hpp"

RedirectInfo::RedirectInfo() : status_code(0) {}

RedirectInfo::RedirectInfo(int code, const std::string &target)
		: status_code(code), url(target)
{
}

bool RedirectInfo::isValid() const { return status_code != 0; }

SizeValue::SizeValue() : _bytes(0) {}

SizeValue::SizeValue(const std::string &sizeStr) { parseSize(sizeStr); }

void SizeValue::parseSize(const std::string &sizeStr)
{
	if (sizeStr.empty())
	{
		_bytes = 0;
		return;
	}
	size_t value = 0;
	size_t i		 = 0;

	while (i < sizeStr.size() && std::isdigit(sizeStr[i]))
	{
		value = value * 10 + (sizeStr[i] - '0');
		i++;
	}
	_bytes = value;

	if (i < sizeStr.size())
	{
		char suffix = sizeStr[i];
		switch (suffix)
		{
		case 'K':
		case 'k':
			_bytes *= 1024;
			break;
		case 'M':
		case 'm':
			_bytes *= 1024 * 1024;
			break;
		case 'G':
		case 'g':
			_bytes *= 1024 * 1024 * 1024;
			break;
		default:
			throw std::invalid_argument("invalid size suffix: " +
																	std::string(1, suffix));
		}
	}
}

size_t SizeValue::getBytes() const { return _bytes; }

bool SizeValue::operator<(const SizeValue &other) const
{
	return _bytes < other._bytes;
}
bool SizeValue::operator>(const SizeValue &other) const
{
	return _bytes > other._bytes;
}
bool SizeValue::operator==(const SizeValue &other) const
{
	return _bytes == other._bytes;
}
