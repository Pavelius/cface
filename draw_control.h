#include "color.h"
#include "rect.h"

#pragma once

enum dock_s {
	DockLeft, DockLeftBottom,
	DockRight, DockRightBottom,
	DockBottom, DockWorkspace,
};
enum command_view_s {
	ViewIcon, ViewIconAndText, ViewText, HideToolbar,
	HideCommand,
};

struct xsfield;

#define CONTROL_PAR(name) {"", "", 0, name::commands}
#define CONTROL_KEY(cpr, key) {#cpr, "", (command::proc)&cpr, 0, {key}, 0, HideCommand}
#define CONTROL_ICN(cpr, text, key, icon) {#cpr, text, (command::proc)&cpr, 0, {key}, icon, ViewIcon}

namespace draw
{
	struct control
	{
		struct plugin
		{
			control&			element;
			plugin*				next;
			static plugin*		first;
			plugin(control& value);
		};
		struct command
		{
			typedef unsigned(control::*proc)(bool run);
			const char*			id;
			const char*			label;
			proc				type;
			command*			child;
			unsigned			key[2];
			int					icon;
			command_view_s		view;
			//
			operator bool() const { return id != 0; }
			const command*		find(const char* id) const;
			const command*		find(int id) const;
		};
		const char*				id;
		dock_s					dock;
		bool					disabled;
		bool					focused;
		bool					show_background;
		bool					show_border;
		bool					show_toolbar;
		//
		control::control() : id(0), dock(DockWorkspace),
			show_border(true), show_background(true), show_toolbar(true),
			disabled(false), focused(false) {}
		//
		virtual void			background(rect& rc);
		virtual void			contextmenu() {}
		unsigned				execute(const char* id, bool run = true);
		color					getcolor(color normal) const;
		virtual control*		getcontrol(const char* id) { return 0; }
		virtual const command*	getcommands() const { return 0; }
		virtual int				geticon(const command& e) const { return e.icon; }
		virtual char*			getdescription(char* result) const;
		virtual const xsfield*	getmeta() const { return 0; }
		virtual char*			getname(char* result) const;
		virtual void*			getobject() { return this; }
		void					invoke(const char* name) const;
		void					keyinput(int id);
		virtual void			nonclient(rect rc);
		bool					open(rect rc);
		bool					open(const char* title, unsigned state, int width, int height);
		bool					open(const char* title);
		int						render(int x, int y, int width, unsigned flags, const command& e) const;
		int						render(int x, int y, int width, const command* commands) const;
		virtual void			redraw(rect rc) {}
		virtual void			prerender() {}
		void					view(rect rc, bool show_toolbar = false);
	};
	struct menu
	{
		unsigned				count;
		bool					separator;
		void*					handle;
		menu();
		~menu();
		void					add(const char* label, unsigned data, bool disabled = false, bool checked = false);
		void					add(const char* id, draw::control* object);
		void					addseparator();
		unsigned				choose(int x, int y);
	};
	void						dockbar(rect& rc);
	unsigned					getdocked(control** output, unsigned count, dock_s type);
	int							view(rect rc, control** pages, int count, int& current, bool show_toolbar, unsigned tab_state, int padding);
}
namespace hot
{
	extern draw::control*		source;
}