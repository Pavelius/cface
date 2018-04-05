#include "bsdata.h"

#pragma once

namespace csdata {
struct typeinfo {
	const char*		id;
	const char*		name;
	typeinfo*		parent;
};
typeinfo type_root[];
typeinfo enum_root[];
struct requisit : typeinfo {
	typeinfo*		type;
};
}
struct bsreq typeinfo_type[];
struct bsreq requisit_type[];