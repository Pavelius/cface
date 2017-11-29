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
		void					execute(void(*proc)());
		color					getcolor(color normal) const;
		virtual control*		getcontrol(const char* id) { return 0; }
		virtual const command*	getcommands() const { return 0; }
		virtual int				geticon(const command& e) const { return e.icon; }
		virtual char*			getdescription(char* result) const;
		virtual const xsfield*	getmeta() const { return 0; }
		virtual char*			getname(char* result) const;
		virtual void*			getobject() { return this; }
		void					invoke(const char* name) const;
		virtual void			inputidle() {}
		virtual void			inputsymbol(int id, int symbol) {}
		virtual void			inputtimer() {}
		virtual void			inputupdate() {}
		virtual void			keybackspace(int id) {}
		virtual void			keydelete(int id) {}
		virtual void			keydown(int id) {}
		virtual void			keyend(int id) {}
		virtual void			keyenter(int id) {}
		virtual void			keyescape(int id) {}
		virtual void			keyhome(int id) {}
		virtual void			keyleft(int id) {}
		void					keyinput(int id);
		virtual void			keypagedown(int id) {}
		virtual void			keypageup(int id) {}
		virtual void			keyright(int id) {}
		virtual void			keyspace(int id) {}
		virtual void			keytab(int id) {}
		virtual void			keyup(int id) {}
		virtual void			mouseleft(point position, int id, bool pressed) {}
		virtual void			mouseleftdbl(point position, int id) {}
		virtual void			mousemove(point position, int id) {}
		virtual void			mouseright(point position, int id, bool pressed) {}
		virtual void			mousewheel(point position, int id, int step) {}
		virtual void			nonclient(rect rc);
		bool					open(rect rc);
		bool					open(const char* title, unsigned state, int width, int height);
		bool					open(const char* title);
		virtual void			prerender() {}
		int						render(int x, int y, int width, unsigned flags, const command& e) const;
		int						render(int x, int y, int width, const command* commands) const;
		virtual void			redraw(rect rc) {}
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