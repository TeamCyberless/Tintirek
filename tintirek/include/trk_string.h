/*
 *	trk_string.h
 * 
 *	Tintirek's dynamic string class.
 */


#ifndef TRK_STRING_H
#define TRK_STRING_H


#include <iostream>
#include <cstring>
#include <sstream>

/* Special string class for Tintirek */
class TrkString
{
public:
	/* Default constructor */
	TrkString()
		: data(nullptr)
		, length(0)
	{ }

	/* Construct from C-style string */
	TrkString(const char* String);

	/* Construct from another TrkString */
	TrkString(const TrkString& Other);

	/* Constructor with a range of characters from a C-style string */
	TrkString(const char* Start, const char* End);

	/* Constructor with a single chararacter */
	TrkString(char Character);

	/* Contrustor with reserving a certain capacity */
	TrkString(int Reserve);


	/* Destructor */
	~TrkString() {
		delete[] data;
	}



	/* Convert all characters to lowercase */
	void ToLower();

	/* Convert all characters to uppercase */
	void ToUpper();

	/* Get a pointer to the first character */
	const char* begin() const;

	/* Get a pointer past the last character */
	const char* end() const;

	/* Get the size of the string */
	size_t size() const;

	/* Find a specific character(s) in the string */
	size_t find(const char* chars, size_t startPos = 0) const;

	/* Find the first character not in the given character set */
	size_t find_first_not_of(const char* chars) const;

	/* Find the last character not in the given character set */
	size_t find_last_not_of(const char* chars) const;

	/* Extract a substring from the string */
	TrkString substr(size_t pos, size_t count = npos) const;


	/* Assignment operator */
	TrkString& operator=(const TrkString& other);

	/* Equality operator with a C-style string */
	bool operator==(const char* str) const;

	/* Inequality operator with a C-style string */
	bool operator!=(const char* str) const;

	/* Conversion operator to char* */
	operator char* ();

	/* Coversion operator to const char* */
	operator const char* () const;



	/* Stream insertion operator for appending values */
	template<typename T>
	TrkString& operator << (const T& value)
	{
		std::stringstream ss;

		if (data != nullptr)
		{
			ss << data;
		}

		ss << value;
		*this = ss.str().c_str();
		return *this;
	}


private:
	/* Stores string data (C-style string) */
	char* data;

	/* Length of the string */
	size_t length;


/* Static constants */
public:
	/* Null position number */
	static const size_t npos = static_cast<size_t>(-1);

	/* Static method for converting a string to an integer */
	static int stoi(const TrkString& str, int base = 10);
};


#endif /* TRK_STRING_H */