#include "adat.h"
#include "string.h"

#pragma once

struct stringkey
{
	const char*		id;
	const char*		text;
};
struct stringlib : adat<stringkey, 32>
{
	adat<char, 2048>	buffer;
	stringlib();
	stringlib(const char* source);
	operator bool() const { return count != 0; }
	const char*			gettext(const char* id) const;
	char*				header(char* result, const char* id, ...) const;
	void				parse(const char* source);
	char*				print(char* result, const char* id, ...) const;
	char*				printn(char* result, const char* id, ...) const;
	char*				printv(char* result, const char* id, const char* param, const char* prefix = 0, const char* postfix = 0) const;
	const char*			puttext(const char* p1, const char* p2);
	const stringkey*	find(const char* id) const;
};