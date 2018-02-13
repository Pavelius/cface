#include "bsdata.h"
#include "stringtree.h"

#pragma once

struct bsdoc {
	const char		*p, *porigin;
	char			operation[2];
	char			buffer[128 * 256];
	stringtree*		comments;
	bsdoc();
	void			generate();
};
