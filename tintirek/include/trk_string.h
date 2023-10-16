/*
 *	trk_string.h
 * 
 *	Tintirek's dynamic string class.
 */

#ifndef TRK_STRING_H
#define TRK_STRING_H

#include "trk_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Special string structure for Tintirek */
typedef struct trk_string_t
{
	/* Stores string data (C-style string) */
	char* data;
	/* Length of the string */
	long long length;
} trk_string_t;


/* Function to create a empty trk_string_t from C-style string */
trk_string_t trk_string_create_empty();


/* Function to create a trk_string_t from C-style string */
trk_string_t trk_string_create(const char* string);


/* Function to create a trk_string_t. Assigned from C-style string */
void trk_string_assign(trk_string_t* other, const char* string);


/* Function to create a trk_string_t from another trk_string_t */
trk_string_t trk_string_duplicate(const trk_string_t* other);


/* Function to create a trk_string_t with a range of characters from a C-style string */
trk_string_t trk_string_between(const char* start, const char* end);


/* Function to create a trk_string_t with a single chararacter */
trk_string_t trk_string_create_char(char character);


/* Function to create a trk_string_t with reserving a certain capacity */
trk_string_t trk_string_reserve(int reserve);


/* Function to destruct a trk_string_t */
void trk_string_destroy(trk_string_t* other);


/* Function to convert all characters to lowercase */
void trk_string_to_lower(trk_string_t* str);


/* Function to convert all characters to uppercase */
void trk_string_to_upper(trk_string_t* str);


/* Function to get a pointer to the first character */
const char* trk_string_begin(const trk_string_t* str);


/* Function to get a pointer past the last character */
const char* trk_string_end(const trk_string_t* str);


/* Function to get the first character */
char trk_string_first(const trk_string_t* str);


/* Function to get the last character */
char trk_string_last(const trk_string_t* str);


/* Function to check if the string starts with the specified prefix */
int trk_string_starts_with(const trk_string_t* str, const char* prefix);


/* Function to get the size of the string */
unsigned int trk_string_size(const trk_string_t* str);


/* Function to find a specific character(s) in the string */
unsigned int trk_string_find(const trk_string_t* str, const char* chars, unsigned int startPos);


/* Function to find the first character not in the given character set */
unsigned int trk_string_find_first_not_of(const trk_string_t* str, const char* chars);


/* Function to find the last character not in the given character set */
unsigned int trk_string_find_last_not_of(const trk_string_t* str, const char* chars);


/* Function to extract a substring from the string */
trk_string_t trk_string_substr(const trk_string_t* str, unsigned int pos, unsigned int count);


/* Function to erase a portion of the string */
void trk_string_erase(trk_string_t* str, unsigned int startPos, unsigned int count);


/* Function to append a C-style string to a trk_string_t */
void trk_string_append_cstr(trk_string_t* prefix, trk_string_t suffix);


/* Null position number */
static const unsigned int npos = -1;

/* Static method for converting a string to an integer */
static int stoi(const trk_string_t* str, int base);


#ifdef __cplusplus
}
#endif

#endif /* TRK_STRING_H */