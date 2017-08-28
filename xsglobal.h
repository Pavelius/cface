#include "xsref.h"

#pragma once

struct xsglobal : xsref
{
	const char*			id;
	xsglobal*			next;
	static xsglobal*	first;
	xsglobal(const char* id, void* object, const xsfield* fields);
	static xsref		getref(const char* id);
	static xsglobal*	find(const char* id);
};
#define BSGLOBAL(c, object) xsglobal c##_manager(#c, object, c##_type);