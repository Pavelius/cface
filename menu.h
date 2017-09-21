#include "draw.h"

#pragma once

// Universal data structure
struct menu
{
	unsigned			count;
	bool				separator;
	void*				handle;
	menu();
	~menu();
	void				add(const char* label, unsigned data, bool disabled = false, bool checked = false);
	void				add(const char* id, draw::context* object);
	void				addseparator();
	unsigned			choose(int x, int y);
};