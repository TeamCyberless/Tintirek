/*
 *	map.cpp
 *
 *	Tintirek's dynamic mapping list class.
 */

#include "trk_map.h"


bool TrkMap::Insert(TrkMapValue* Value)
{
	TrkMapValue* existing_value = Find(Value->key);
	if (existing_value)
	{
		TrkMapValue* next = existing_value->next_value;
		Value->next_value = next;
		existing_value = Value;
		return true;
	}

	Value->next_value = head;
	head = Value;
	return true;
}

bool TrkMap::Erase(const TrkString key)
{
	TrkMapValue *current = head, *previous = nullptr;

	while (current != nullptr)
	{
		if (current->key == key)
		{
			if (previous)
			{
				previous->next_value = current->next_value;
			}
			else
			{
				head = current->next_value;
			}
			delete current;
			return true;
		}

		previous = current;
		current = current->next_value;
	}

	return false;
}

void TrkMap::Empty()
{
	TrkMapValue* current = head;
	while (current != nullptr)
	{
		TrkMapValue* next = current->next_value;
		delete current;
		current = next;
	}
	head = nullptr;
}

int TrkMap::Size() const
{
	int count = 0;
	TrkMapValue* current = head;
	while (current != nullptr)
	{
		count++;
		current = current->next_value;
	}
	return count;
}

TrkMapValue* TrkMap::Find(const TrkString key)
{
	TrkMapValue* current = head;
	while (current != nullptr)
	{
		if (current->key == key)
			return current;
		current = current->next_value;
	}
	return current;
}

TrkString TrkMapValue::GetKey() const
{
	return key;
}

TrkMapValue* TrkMapValue::GetNext() const
{
	return next_value;
}

void TrkMapValue::SetNext(TrkMapValue* Next)
{
	next_value = Next;
}

int TrkMapValue::GetType() const
{
	return type;
}

bool TrkMapValue::GetBool() const
{
	return *reinterpret_cast<const bool*>(value);
}

char TrkMapValue::GetChar() const
{
	return *reinterpret_cast<const char*>(value);
}

int TrkMapValue::GetInt() const
{
	return *reinterpret_cast<const int*>(value);
}

long TrkMapValue::GetInt64() const
{
	return *reinterpret_cast<const long*>(value);
}

float TrkMapValue::GetFloat() const
{
	return *reinterpret_cast<const float*>(value);
}

double TrkMapValue::GetDouble() const
{
	return *reinterpret_cast<const double*>(value);
}

TrkString TrkMapValue::GetString() const
{
	return *reinterpret_cast<const TrkString*>(value);
}