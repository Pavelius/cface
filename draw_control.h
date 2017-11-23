#include "color.h"
#include "rect.h"
#include "wrapper.h"

#pragma once

enum dock_s {
	DockLeft, DockLeftBottom,
	DockRight, DockRightBottom,
	DockBottom, DockWorkspace,
};

namespace draw
{
	struct control : wrapper
	{
		struct plugin
		{
			control&		element;
			plugin(control& value);
			plugin*			next;
			static plugin*	first;
		};
		const char*			id;
		dock_s				dock;
		bool				disabled;
		bool				focused;
		bool				show_background;
		bool				show_border;
		bool				show_toolbar;
		//
		control::control() : id(0), dock(DockWorkspace),
			show_border(true), show_background(true), show_toolbar(true),
			disabled(false), focused(false) {}
		//
		virtual void		background(rect& rc);
		virtual void		contextmenu() {}
		color				getcolor(color normal) const;
		virtual char*		getdescription(char* result) const;
		virtual char*		getname(char* result) const;
		void				invoke(const char* name) const;
		virtual void		nonclient(rect rc);
		bool				open(rect rc);
		bool				open(const char* title, unsigned state, int width, int height);
		int					render(int x, int y, int width, unsigned flags, const wrapper::command& e) const;
		int					render(int x, int y, int width, const wrapper::command* commands) const;
		virtual void		redraw(rect rc) {}
		virtual void		prerender() {}
		void				view(rect rc, bool show_toolbar = false);
	};
	void					dockbar(rect& rc);
	unsigned				getdocked(control** output, unsigned count, dock_s type);
	int						view(rect rc, control** pages, int count, int& current, bool show_toolbar, unsigned tab_state, int padding);
}