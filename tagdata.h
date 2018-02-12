#include "collections.h"

#pragma once

struct tagdata {
	arefc<unsigned> indecies;
	arefc<char> strings;
	int	add(const char* name);
	void clear();
	int	 find(const char* name);
	const char* get(int id) const;
};