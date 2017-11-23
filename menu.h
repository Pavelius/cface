#include "draw_control.h"

#pragma once

struct menu
{
	unsigned			count;
	bool				separator;
	void*				handle;
	menu();
	~menu();
	void				add(const char* label, unsigned data, bool disabled = false, bool checked = false);
	void				add(const char* id, draw::control* object);
	void				addseparator();
	unsigned			choose(int x, int y);
};