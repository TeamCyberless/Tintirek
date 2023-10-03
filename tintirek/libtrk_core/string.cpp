/*
 *	string.cpp
 *
 *	Tintirek's dynamic string class.
 */

#include "trk_string.h"

TrkString::TrkString(const char* String)
{
	length = std::strlen(String);
	data = new char[length + 1];
	std::strcpy(data, String);
}

TrkString::TrkString(const TrkString& Other)
{
	length = Other.length;
	data = new char[length + 1];
	std::strcpy(data, Other.data);
}

TrkString::TrkString(const char* Start, const char* End)
{
	length = End - Start;
	data = new char[length + 1];
	std::memcpy(data, Start, length);
	data[length] = '\0';
}

TrkString::TrkString(char Character)
{
	length = 1;
	data = new char[2];
	data[0] = Character;
	data[1] = '\0';
}

TrkString::TrkString(int Reserve)
{
	length = 0;
	data = new char[Reserve];
	data[0] = '\0';
}

void TrkString::ToLower()
{
	for (size_t i = 0; i < length; ++i)
	{
		data[i] = std::tolower(data[i]);
	}
}

void TrkString::ToUpper()
{
	for (size_t i = 0; i < length; ++i)
	{
		data[i] = std::toupper(data[i]);
	}
}

const char* TrkString::begin() const
{
	return data;
}

const char* TrkString::end() const
{
	return data + length;
}

char TrkString::first() const
{
	if (length > 0)
	{
		return data[0];
	}

	return '\0';
}

char TrkString::last() const
{
	if (length > 0)
	{
		return data[length - 1];
	}

	return '\0';
}

bool TrkString::startswith(const TrkString& prefix) const
{
	size_t prefixLength = prefix.size();
	if (length < prefixLength)
	{
		return false;
	}

	return (std::strncmp(data, prefix.data, prefixLength) == 0);
}

size_t TrkString::size() const
{
	return length;
}

size_t TrkString::find(const char* chars, size_t startPos) const
{
	if (startPos >= length)
	{
		return npos;
	}

	const char* result = std::strstr(data + startPos, chars);
	if (result == nullptr)
	{
		return npos;
	}

	return result - data;
}

size_t TrkString::find_first_not_of(const char* chars) const
{
	if (chars == nullptr) return npos;
	for (size_t i = 0; i < length; ++i)
	{
		if (std::strchr(chars, data[i]) == nullptr)
		{
			return i;
		}
	}

	return npos;
}

size_t TrkString::find_last_not_of(const char* chars) const
{
	if (chars == nullptr) return npos;
	for (size_t i = length - 1; i != npos; --i)
	{
		if (std::strchr(chars, data[i]) == nullptr)
		{
			return i;
		}
	}

	return npos;
}

TrkString TrkString::substr(size_t pos, size_t count) const
{
	if (pos >= length)
	{
		return TrkString();
	}

	if (count == npos || (pos + count) > length)
	{
		count = length - pos;
	}

	TrkString result;
	result.length = count;
	result.data = new char[count + 1];
	std::strncpy(result.data, data + pos, count);
	result.data[count] = '\0';

	return result;
}

TrkString& TrkString::operator=(const TrkString& other)
{
	if (this == &other) return *this;

	delete[] data;
	length = other.length;
	data = new char[length + 1];
	std::strcpy(data, other.data);

	return *this;
}

bool TrkString::operator==(const char* str) const
{
	if (str == nullptr && data == nullptr)
	{
		return true;
	}
	else if (str == nullptr || data == nullptr)
	{
		return false;
	}

	return std::strcmp(data, str) == 0;
}

bool TrkString::operator!=(const char* str) const
{
	if (str == nullptr && data == nullptr)
	{
		return false;
	}
	else if (str == nullptr || data == nullptr)
	{
		return true;
	}

	return std::strcmp(data, str) != 0;
}

TrkString::operator char* ()
{
	return data;
}

TrkString::operator const char* () const
{
	return data;
}

std::ostream& operator << (std::ostream& os, const TrkString& str)
{
	os << str;
	return os;
}

int TrkString::stoi(const TrkString& str, int base)
{
	return std::strtol(str.data, nullptr, base);
}