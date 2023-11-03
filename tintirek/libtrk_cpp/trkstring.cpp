/*
 *	trkstring.cpp
 *
 *	Tintirek's dynamic string class.
 */

#include "trkstring.h"
#include <cstring>
#include <string.h>

TrkString::TrkString()
{
	data.length = 0;
	data.data = static_cast<char*>(malloc(1));
	if (data.data != NULL)
	{
		data.data[0] = '\0';
	}
}

TrkString::TrkString(const trk_string_t& value)
{
	data.length = value.length;
	data.data = static_cast<char*>(malloc(value.length + 1));
	if (data.data != NULL)
	{
		std::strcpy(data.data, value.data);
	}
}

TrkString::TrkString(trk_string_t* value)
{
	if (value != nullptr) {
        data.length = value->length;
		data.data = static_cast<char*>(malloc(value->length + 1));
		if (data.data != NULL)
		{
			std::strcpy(data.data, value->data);
		}
    } else {
		data.length = 0;
		data.data = NULL;
    }
}

TrkString::TrkString(const char* String)
{
	data.length = std::strlen(String);
	data.data = static_cast<char*>(malloc(data.length + 1));
	if (data.data != NULL)
	{
		std::strcpy(data.data, String);
	}
}

TrkString::TrkString(const TrkString& Other)
{
	data.length = Other.data.length;
	data.data = static_cast<char*>(malloc(data.length + 1));
	if (data.data != NULL)
	{
		std::strcpy(data.data, Other.data.data);
	}
}

TrkString::TrkString(const char* Start, const char* End)
{
	data.length = End - Start;
	data.data = static_cast<char*>(malloc(data.length + 1));
	if (data.data != NULL)
	{
		std::memcpy(data.data, Start, data.length);
		data.data[data.length] = '\0';
	}
}

TrkString::TrkString(char Character)
{
	data.length = 1;
	data.data = static_cast<char*>(malloc(2));
	if (data.data != NULL)
	{
		data.data[0] = Character;
		data.data[1] = '\0';
	}
}

TrkString::TrkString(int Reserve)
{
	data.length = 0;
	data.data = static_cast<char*>(malloc(Reserve));
	if (data.data != NULL)
	{
		data.data[0] = '\0';
	}
}

TrkString::~TrkString()
{
	if (data.data != NULL)
	{
		free(data.data);
	}
}

void TrkString::ToLower()
{
	for (size_t i = 0; i < data.length; ++i)
	{
		data.data[i] = std::tolower(data.data[i]);
	}
}

void TrkString::ToUpper()
{
	for (size_t i = 0; i < data.length; ++i)
	{
		data.data[i] = std::toupper(data.data[i]);
	}
}

const char* TrkString::c_str() const
{
	return data.data;
}

const char* TrkString::begin() const
{
	return data.data;
}

const char* TrkString::end() const
{
	return data.data + data.length;
}

char TrkString::first() const
{
	if (data.length > 0)
	{
		return data.data[0];
	}

	return '\0';
}

char TrkString::last() const
{
	if (data.length > 0)
	{
		return data.data[data.length - 1];
	}

	return '\0';
}

bool TrkString::startswith(const TrkString& prefix) const
{
	size_t prefixLength = prefix.size();
	if (data.length < prefixLength)
	{
		return false;
	}

	return (std::strncmp(data.data, prefix.data.data, prefixLength) == 0);
}

size_t TrkString::size() const
{
	return data.length;
}

size_t TrkString::find(const char* chars, size_t startPos) const
{
	if (startPos >= data.length)
	{
		return npos;
	}

	const char* result = std::strstr(data.data + startPos, chars);
	if (result == nullptr)
	{
		return npos;
	}

	return result - data.data;
}

size_t TrkString::find_first_not_of(const char* chars) const
{
	if (chars == nullptr) return npos;
	for (size_t i = 0; i < data.length; ++i)
	{
		if (std::strchr(chars, data.data[i]) == nullptr)
		{
			return i;
		}
	}

	return npos;
}

size_t TrkString::find_last_not_of(const char* chars) const
{
	if (chars == nullptr) return npos;
	for (size_t i = data.length - 1; i != npos; --i)
	{
		if (std::strchr(chars, data.data[i]) == nullptr)
		{
			return i;
		}
	}

	return npos;
}

TrkString TrkString::substr(size_t pos, size_t count) const
{
	if (pos >= data.length)
	{
		return TrkString();
	}

	if (count == npos || (pos + count) > data.length)
	{
		count = data.length - pos;
	}

	TrkString result;
	free(result.data.data);

	result.data.length = count;
	result.data.data = new char[count + 1];
	std::strncpy(result.data.data, data.data + pos, count);
	result.data.data[count] = '\0';

	return result;
}

void TrkString::erase(size_t startPos, size_t count)
{
	if (startPos >= data.length) {
		return;
	}

	if (count == npos) {
		count = data.length - startPos;
	}

	if (startPos + count > data.length) {
		count = data.length - startPos;
	}

	for (size_t i = startPos; i < data.length - count; ++i) {
		data.data[i] = data.data[i + count];
	}

	for (size_t i = data.length - count; i < data.length; ++i) {
		data.data[i] = '\0';
	}

	data.length -= count;
}

TrkString& TrkString::operator=(const TrkString& other)
{
	if (this == &other) return *this;

	delete[] data.data;
	data.length = other.data.length;
	data.data = new char[data.length + 1];
	
	if (other.data.data != nullptr)
	{
		std::strcpy(data.data, other.data.data);
	}
	else
	{
		std::strcpy(data.data, "\0");
	}

	return *this;
}

TrkString TrkString::operator+(const TrkString& other)
{
	char* result = new char[data.length + other.data.length + 1];
	strcpy(result, data.data);
	strcat(result, other.data.data);
	return TrkString(result);
}

bool TrkString::operator==(const char* str) const
{
	if (str == nullptr && data.data == nullptr)
	{
		return true;
	}
	else if (str == nullptr || data.data == nullptr)
	{
		return false;
	}

	return std::strcmp(data.data, str) == 0;
}

bool TrkString::operator!=(const char* str) const
{
	if (str == nullptr && data.data == nullptr)
	{
		return false;
	}
	else if (str == nullptr || data.data == nullptr)
	{
		return true;
	}

	return std::strcmp(data.data, str) != 0;
}

bool TrkString::operator==(TrkString str) const
{
	return TrkString::operator==(str.data.data);
}

bool TrkString::operator!=(TrkString str) const
{
	return TrkString::operator!=(str.data.data);
}

bool TrkString::operator<(const TrkString& rhs) const
{
	return std::strcmp(data.data, rhs.data.data) < 0;
}

TrkString::operator char* ()
{
	return data.data;
}

TrkString::operator const char* () const
{
	return data.data;
}

TrkString::operator const trk_string_t* () const
{
	return &data;
}

int TrkString::stoi(const TrkString& str, int base)
{
	return std::strtol(str.data.data, nullptr, base);
}

std::ostream& operator<<(std::ostream& os, const TrkString& str)
{
	os << str.c_str();
	return os;
}

std::ostream& operator<<(std::ostream& stream, const trk_string_t& value)
{
	stream << value.data;
	return stream;
}