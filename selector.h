#pragma once

enum condition_s : unsigned char {
	Equal, NotEqual, Lesser, LesserEqual, Greater, GreaterEqual, Between, InList, NotInList, InHierarhy, Like
};
struct selector
{
	const char*		id;
	condition_s		operation;
	int				value;
	int				value_to;
	operator bool() const { return id != 0; }
	bool			match(int value) const;
	bool			match(const char* value) const;
};