#include "adat.h"
#include "bsdata.h"

#pragma once

namespace database {
struct object {
	const char*		name; // Name for data object. Use szdup().
	object*			parent;
	//
	object&			add(const char* name);
	void			clear();
	static object*	find(const char* uid);
	static object	root;
};
extern bsreq		object_type[];
extern bsdata		object_manager;
extern adat<object, 256 * 256 * 4> objects;
extern object		root;
}