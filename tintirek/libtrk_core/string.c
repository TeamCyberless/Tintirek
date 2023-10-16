/*
 *	string.c
 *
 *	Tintirek's dynamic string class.
 */

#include "trk_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


trk_string_t trk_string_create_empty()
{
	trk_string_t result;
	result.data = NULL;
	result.length = 0;
	return result;
}

trk_string_t trk_string_create(const char* string)
{
	trk_string_t result;
	result.data = strdup(string);
	result.length = strlen(string);
	return result;
}

void trk_string_assign(trk_string_t* other, const char* string)
{
	if (other != NULL)
	{
		trk_string_destroy(other);
	}

	trk_string_t result;
	result.data = strdup(string);
	result.length = strlen(string);
	other = &result;
}

trk_string_t trk_string_duplicate(const trk_string_t* other)
{
	trk_string_t result;
	result.data = strdup(other->data);
	result.length = other->length;
	return result;
}

trk_string_t trk_string_between(const char* start, const char* end)
{
	unsigned int length = end - start;
	trk_string_t result;
	result.data = (char*)malloc(length + 1);
	strncpy(result.data, start, length);
	result.data[length] = '\0';
	result.length = length;
	return result;
}

trk_string_t trk_string_create_char(char character)
{
	trk_string_t result;
	result.data = (char*)malloc(2);
	result.data[0] = character;
	result.data[1] = '\0';
	result.length = 1;
	return result;
}

trk_string_t trk_string_reserve(int reserve)
{
	trk_string_t result;
	result.data = (char*)malloc(reserve + 1);
	result.data[0] = '\0';
	result.length = 0;
	return result;
}

void trk_string_destroy(trk_string_t* other)
{
	free(other->data);
	other->data = NULL;
	other->length = 0;
}

void trk_string_to_lower(trk_string_t* str)
{
	for (unsigned int i = 0; i < str->length; ++i)
	{
		str->data[i] = tolower(str->data[i]);
	}
}

void trk_string_to_upper(trk_string_t* str)
{
	for (unsigned int i = 0; i < str->length; ++i)
	{
		str->data[i] = toupper(str->data[i]);
	}
}

const char* trk_string_begin(const trk_string_t* str)
{
	return str->data;
}

const char* trk_string_end(const trk_string_t* str)
{
	return str->data + str->length;
}

char trk_string_first(const trk_string_t* str)
{
	return (str->length > 0) ? str->data[0] : '\0';
}

char trk_string_last(const trk_string_t* str)
{
	return (str->length > 0) ? str->data[str->length - 1] : '\0';
}

trk_boolean_t trk_string_starts_with(const trk_string_t* str, const char* prefix)
{
	unsigned int prefixLength = strlen(prefix);
	if (str->length < prefixLength)
	{
		return FALSE;
	}

	return strncmp(str->data, prefix, prefixLength) == 0;
}

unsigned int trk_string_size(const trk_string_t* str)
{
	return str->length;
}

unsigned int trk_string_find(const trk_string_t* str, const char* chars, unsigned int startPos)
{
	const char* result = str->data + startPos;
	result = strpbrk(result, chars);
	if (result == NULL)
	{
		return npos;
	}

	return result - str->data;
}

unsigned int trk_string_find_first_not_of(const trk_string_t* str, const char* chars)
{
	const char* result = str->data;
	while (*result && strchr(chars, *result))
	{
		++result;
	}

	return (result == str->data) ? npos : result - str->data;
}

unsigned int trk_string_find_last_not_of(const trk_string_t* str, const char* chars)
{
	const char* result = str->data + str->length - 1;
	while (result >= str->data && strchr(chars, *result))
	{
		--result;
	}

	return (result < str->data) ? npos : result - str->data;
}

trk_string_t trk_string_substr(const trk_string_t* str, unsigned int pos, unsigned int count)
{
	if (pos >= str->length)
	{
		return trk_string_create_empty();
	}

	if (count == npos || (pos + count) > str->length)
	{
		count = str->length - pos;
	}

	return trk_string_between(str->data + pos, str->data + pos + count);
}

void trk_string_erase(trk_string_t* str, unsigned int startPos, unsigned int count)
{
	if (startPos >= str->length) {
		return;
	}

	if (count == npos || (startPos + count) > str->length) {
		count = str->length - startPos;
	}

	memmove(str->data + startPos, str->data + startPos + count, str->length - startPos - count);
	str->length -= count;
	str->data[str->length] = '\0';
}

void trk_string_append_cstr(trk_string_t* prefix, trk_string_t suffix)
{
	unsigned int new_length = prefix->length + suffix.length;
	prefix->data = (char*)realloc(prefix->data, new_length + 1);
	strncpy(prefix->data + prefix->length, suffix.data, suffix.length);
	prefix->data[new_length] = '\0';
	prefix->length = new_length;
}

int stoi(const trk_string_t* str, int base)
{
	return strtol(str->data, NULL, base);
}