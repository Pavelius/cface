#pragma once

struct wrapper
{
	struct command
	{
		typedef unsigned(*proc)(wrapper* object, bool run);
		const char*			id;
		const char*			label;
		proc				type;
		command*			child;
		unsigned			key[2];
		//
		operator bool() const { return id != 0; }
		const command*		find(const char* id) const;
		const command*		find(int id) const;
	};
	virtual unsigned		execute(const char* id, bool run);
	virtual const command*	getcommands() const { return 0; }
	virtual const struct xsfield* getmeta() const { return 0; }
	virtual void*			getobject() { return this; }
};
