#include "stringtree.h"
#include "xsbase.h"

#pragma once

struct xsdoc
{
	const char		*p, *porigin;
	char			operation[2];
	char			buffer[128 * 256];
	stringtree*		comments;
	xsdoc();
	virtual void	addcomment(xsbase* m, const xsfield* fd) {}
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
