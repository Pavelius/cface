#include "bsdata.h"
#include "stringtree.h"

#pragma once

struct bsdoc
{
	const char		*p, *porigin;
	char			operation[2];
	char			buffer[128 * 256];
	stringtree*		comments;
	bsdoc();
	virtual void	addcomment(bsdata* m, const bsreq* fd) {}
	void			generate();
	bool			parsecpp(const char* path, const char* name);
	bool			skipkeyword();
	bool			skipcm();
	void			skipline();
	bool			skipoper();
	bool			skipprep();
	bool			skipstring();
	void			skipws();
};
