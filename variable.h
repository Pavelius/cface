#pragma once

struct variable
{
	void*			object;
	unsigned		size;
	//
	operator bool() const { return object != 0; }
	void			clear();
	int				get() const;
	void			set(int value);
};
