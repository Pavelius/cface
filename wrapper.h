#pragma once

struct xsfield;

enum command_view_s {
	ViewIcon, ViewIconAndText, ViewText, HideToolbar,
	HideCommand,
};

struct wrapper
{
	struct command
	{
		typedef unsigned(*proc)(wrapper* object, bool run);
		const char*				id;
		const char*				label;
		proc					type;
		command*				child;
		unsigned				key[2];
		int						icon;
		command_view_s			view;
		//
		operator bool() const { return id != 0; }
		const command*			find(const char* id) const;
		const command*			find(int id) const;
	};
	virtual unsigned			execute(const char* id, bool run);
	virtual const command*		getcommands() const { return 0; }
	virtual int					geticon(const command& e) const { return e.icon; }
	virtual const xsfield*		getmeta() const { return 0; }
	virtual void*				getobject() { return this; }
	virtual wrapper*			getwrapper(const char* id) { return 0; }
};
namespace hot
{
	extern wrapper*				source;
}
