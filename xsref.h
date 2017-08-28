#include "xsfield.h"

#pragma once

// Metadata reference object
struct xsref
{
	const xsfield*	fields;
	void*			object;
	//
	inline operator bool() const { return fields != 0; }
	//
	void			add(const char* id, int value) { set(id, get(id) + value); }
	void			clear() const;
	int				get(const char* id) const;
	int				get(const char* id, unsigned index) const;
	int				getdf(const char* id, const int default_value) const;
	const char*		getdf(const char* id, const char* default_value) const;
	xsref			getr(const char* id, unsigned index = 0) const;
	int				get() const { return fields ? fields->get(object) : 0; }
	xsref			getvalue(const char* id, unsigned index = 0) const;
	bool			isempthy() const;
	const void*		ptr(const char* id, unsigned index = 0) const;
	void			set(int value) { if(fields) fields->set(object, value); }
	void			set(const char* id, int value, unsigned index = 0);
};