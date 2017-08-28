#include "aref.h"

#pragma once

struct tagdata
{
	aref<int>		indecies;
	aref<char>		strings;
	tagdata();
	~tagdata();
	int				add(const char* name);
	void			clear();
	int				find(const char* name);
	const char*		get(int id) const;
};