/*
 *	trkstring.h
 * 
 *	Tintirek's dynamic string class.
 */

#ifndef TRK_STRING_CPP_H
#define TRK_STRING_CPP_H

#include "trk_string.h"
#include <sstream>


/* Special string class for Tintirek */
class TrkString : public trk_string_t
{
public:
    /* Default constructor */
	TrkString();

	/* Construct from trk_string_t structure */
	TrkString(const trk_string_t& value);
	/* Construct from trk_string_t pointer structure */
	TrkString(trk_string_t* value);
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
	~TrkString();

	/* Convert all characters to lowercase */
	void ToLower();
	/* Convert all characters to uppercase */
	void ToUpper();

	/* Returns a pointer to referenced to string */
	const char* c_str() const;
	/* Get a pointer to the first character */
	const char* begin() const;
	/* Get a pointer past the last character */
	const char* end() const;

	/* Get first character */
	char first() const;
	/* Get last character */
	char last() const;

	/* Checks if the string starts with the specified prefix */
	bool startswith(const TrkString& prefix) const;

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

	/* Erase a portion of the string */
	void erase(size_t startPos = 0, size_t count = npos);


	/* Assignment operator */
	TrkString& operator=(const TrkString& other);
	/* Addition operator */
	TrkString operator+(const TrkString& other);
	/* Equality operator with a C-style string */
	bool operator==(const char* str) const;
	/* Inequality operator with a C-style string */
	bool operator!=(const char* str) const;
	/* Equality operator with a Tintirek's string */
	bool operator==(TrkString str) const;
	/* Inequality operator with Tintirek's string */
	bool operator!=(TrkString str) const;
	/* Less than operator with Tintirek's strings */
	bool operator<(const TrkString& rhs) const;
	/* Conversion operator to char* */
	operator char* ();
	/* Coversion operator to const char* */
	operator const char* () const;
	/* Conversion operator to trk_string_t* */
	operator const trk_string_t* () const;


	/* Stream insertion operator for appending values */
	template<typename T>
	TrkString& operator << (const T& value)
	{
		std::stringstream ss;

		if (data.data != nullptr)
		{
			ss << data.data;
		}

		ss << value;
		
		*this = ss.str().c_str();
		return *this;
	}

protected:
	/* Internal string system */
	trk_string_t data;

public:
	/* Null position number */
	static const size_t npos = static_cast<size_t>(-1);

	/* Static method for converting a string to an integer */
	static int stoi(const TrkString& str, int base = 10);
};

/* Override for streams */
std::ostream& operator<<(std::ostream& os, const TrkString& str);
std::ostream& operator<<(std::ostream& stream, const trk_string_t& value);


#endif /* TRK_STRING_CPP_H */