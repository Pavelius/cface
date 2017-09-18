#pragma once

struct variable
{
	void*			object;
	unsigned		size;
	void			clear();
	int				get() const;
	void			set(int value);
};
