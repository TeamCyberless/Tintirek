/*
 *	trk_string.h
 *
 *	Tintirek's dynamic mapping list class.
 */

#ifndef TRK_MAP_H
#define TRK_MAP_H

#include <cstdint>

#include "trk_string.h"


/* Special map class for Tintirek */
class TrkMap
{
	/* Forward friend declaration for TrkMapValue */
	friend class TrkMapValue;

public:
	/* Default constructor */
	TrkMap() 
		: head(nullptr) 
	{ }

	/* Destructor */
	~TrkMap() 
	{
		Empty(); 
	}


	/* Insert a key-value pair into the map */
	bool Insert(TrkMapValue* Value);
	/* Erase a key-value pair from the map using the key */
	bool Erase(const TrkString key);
	/* Empty the map and release associated memory */
	void Empty();
	/* Get the number of key-value pairs in the map */
	int Size() const;
	/* Find a key in the map and return its associated value */
	TrkMapValue* Find(const TrkString key);
	
	/* Value types of map values */
	enum ValueType : uint8_t
	{
		BOOL = 0,
		CHAR = 1,
		INTEGER = 2,
		INTEGER64 = 3,
		FLOAT = 4,
		DOUBLE = 5,
		STRING = 6
	};

private:
	TrkMapValue* head;
};

/* Special value class for Tintirek's map class */
class TrkMapValue
{
	/* Forward friend declaration for TrkMap */
	friend class TrkMap;

public:
	/* Default constructor */
	TrkMapValue(const TrkString Key, const void* Value, const int Type)
		: value(Value)
		, type(Type)
		, next_value(nullptr)
		, key(Key)
	{ }

	/* Destructor */
	~TrkMapValue()
	{
		if (next_value)
		{
			delete next_value;
		}
	}

	/* Get the key associated with this map value */
	TrkString GetKey() const;
	/* Get a pointer to the next map value in the list */
	TrkMapValue* GetNext() const;
	/* Set the next map value in the list */
	void SetNext(TrkMapValue* Next);
	/* Get the type of this map value (e.g., BOOL, CHAR, INTEGER, etc.) */
	int GetType() const;

	/* Get the boolean value stored in this map value */
	bool GetBool() const;
	/* Get the character value stored in this map value */
	char GetChar() const;
	/* Get the integer value stored in this map value */
	int GetInt() const;
	/* Get the 64 bits integer value stored in this map value */
	long GetInt64() const;
	/* Get the floating value value stored in this map value */
	float GetFloat() const;
	/* Get the 64 bits floating value stored in this map value */
	double GetDouble() const;
	/* Get the string value stored in this map value */
	TrkString GetString() const;

	/*
	 *	Type conversion operators for easy value retrieval
	 */
	operator bool() const { return GetBool(); }
	operator char() const { return GetChar(); }
	operator int() const { return GetInt(); }
	operator long() const { return GetInt64(); }
	operator float() const { return GetFloat(); }
	operator double() const { return GetDouble(); }
	operator TrkString() const { return GetString(); }

private:
	TrkString key;
	TrkMapValue* next_value;
	const void* value;
	const int type;
};


#endif /* TRK_MAP_H */