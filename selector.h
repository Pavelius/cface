#pragma once

enum selector_flags {
	SelectorNotUsable = 1,
	SelectorHide = 2,
};

struct selector
{
	int				id;
	int				operation;
	int				value;
	int				value_to;
	unsigned		flags;
	operator bool() const { return id != 0; }
	bool			match(int value) const;
	bool			match(const char* value) const;
};