#include "collections.h"
#pragma once

struct valelement
{
	int						value;
	unsigned				flags;
	char					name[64 - sizeof(int) * 2];
};

struct vallist : arefc<valelement>
{
	valelement&				add(int value, const char* name, unsigned flags = 0);
};