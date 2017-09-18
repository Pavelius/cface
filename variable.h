#pragma once

struct variable
{
	void*			object;
	unsigned		size;
	//
	operator bool() const { return object != 0; }
	void			clear();
	int				get() const;
	template<class T> void link(T& v) { object = &v; size = sizeof(v); }
	template<class T> void link(T* v) { object = v; size = sizeof(*v); }
	void			set(int value);
};
