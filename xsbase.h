#include "amem.h"
#include "xsref.h"

#pragma once

struct xsbase : amem
{
	const char*			id;
	const xsfield*		fields;
	const void*			clone;
	xsbase*				next;
	static xsbase*		first;
	//
	xsbase(const char* id, void* data, unsigned size, unsigned lenght, const xsfield* fields, const void* clone);
	//
	void*				add(const void* element = 0) override;
	void				clear();
	xsref				getref(int index) const;
	static xsref		getref(const char* id);
	static xsref		getref(const char* id, const xsfield* fields);
	static xsref		getrefbyptr(const void* value);
	static xsbase*		find(const char* id);
	static xsbase*		find(const xsfield* id);
	static xsbase*		findbyptr(const void* object);
	static xsref		find(const char* base, const char* id, const char* value);
	xsref				find(const char* id, const char* value);
	static bool			read(const char* url);
	static bool			write(const char* url);
	static bool			write(const char* url, const char** names);
};
struct xsenum
{
	const char*			id;
	const char*			name;
	const char*			text;
};
#define BSMETA(c) \
static c c##_clone;\
xsbase c##_manager(#c, c##_data, sizeof(c##_data[0]), sizeof(c##_data), c##_type, &c##_clone);

#define BSTYPE(c) \
xsbase c##_manager(#c, 0, 0, 0, c##_type, 0);