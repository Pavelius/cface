#include "xscontext.h"

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
	void				add(const char* id, xscontext* context);
	void				addseparator();
	unsigned			choose(int x, int y);
};