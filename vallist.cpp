#include "crt.h"
#include "vallist.h"

valelement& vallist::add(int value, const char* name, unsigned flags) {
	auto e = (valelement*)amem::add();
	e->value = value;
	e->flags = flags;
	if(!name)
		memset(e->name, 0, sizeof(e->name));
	else
		zcpy(e->name, name, sizeof(e->name) / sizeof(e->name[0]) - 1);
	return *e;
}