#include "xsref.h"

#pragma once

struct xscontext
{
	struct command
	{
		typedef unsigned(*proc)(xscontext* context, bool run);
		const char*			id;
		const char*			label;
		proc				type;
		command*			child;
		unsigned			key[2];
		operator bool() const { return id != 0; }
		command*			find(const char* id);
		command*			findbykey(int id);
	};
	virtual unsigned 		execute(const char* id, bool run);
	virtual command*		getcommands() const { return 0; }
	virtual xsfield*		getmeta() const { return 0; }
	inline xsref			getr() const { return{getmeta(), (void*)this}; }
};