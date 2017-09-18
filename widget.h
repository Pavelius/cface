#include "color.h"
#include "rect.h"
#include "xscontext.h"
#include "xsref.h"

#pragma once

namespace draw
{
	struct element;
	struct control;
	struct element;
	// Widget descriptor
	struct widget
	{
		// Widget procedure
		typedef int(*proc)(int x, int y, int width, element& e);
		// Plugin for widget descriptor
		struct plugin
		{
			struct element
			{
				const char*		id;
				widget::proc	render;
			};
			element*		controls;
			plugin*			next;
			static plugin*	first;
			plugin(element* controls);
			static element*	find(const char* id);
		};
		proc				type; // Control proc
		const char*			id; // Control identification
		const char*			label; // Title text or text appeared in control
		const widget*		childs; // Chilren elements
		unsigned			flags; // Text formation and other flags
		int					width; // width for horizontal parts
		int					height; // height in lines
		int					value; // Integer parameter value
		int					title; // Title width (if 0 then title taken from parent)
		const char*			tips; // Tooltips value
		const char*			link; // Hyperlink value
		unsigned			maximum; // Maximum value
		operator bool() const { return type != 0; }
		void				clear();
		void				decortext() const;
		bool				ischecked() const;
		bool				isdisabled() const;
		bool				isfocused() const;
		color				getcolor(color normal) const;
		color				getcolor(rect rc, color normal, color active) const;
		inline const char*	getdata() const { return link ? link : id; }
		inline int			getheight() const { return height ? height : 1; }
		inline widget&		settips(const char* value) { tips = value; return *this; }
		inline widget&		setvalue(int number) { value = number; return *this; }
	};
	struct element : widget
	{
		control*			context;
		element*			parent;
		xsref				data;
		int					row;
		int					column;
		bool*				separator;
		rect				rectangle;
		//
		element() { memset(this, 0, sizeof(element)); }
		element(const widget& e, element* parent);
		element(control& e);
		//
		void				addbutton(rect& rc, int c1, const char* t1, int k1, const char* tt1);
		void				addbutton(rect& rc, int c1, const char* t1, int k1, const char* tt1, int c2, const char* t2, int k2, const char* tt2);
		void				addvalue(int value);
		bool				choose();
		void				clear();
		bool				editing();
		void				focusing(const rect& rc);
		element*			getparent(widget::proc type);
		int					gettitle();
		char*				getstring(char* temp, bool to_buffer);
		bool				isfocus() const;
		void				setposition(int& x, int& y, int& width);
		bool				editstart(const rect& rc, int ev);
	};
	struct control : xscontext
	{
		struct plugin
		{
			control&		element;
			dock_types		type;
			plugin*			next;
			static plugin*	first;
			plugin(control& element);
			static plugin*	find(const char* id);
		};
		const char*			id;
		bool				show_border;
		bool				show_background;
		bool				show_toolbar;
		bool				disabled;
		bool				focused;
		control();
		virtual void		background(rect& rc);
		virtual void		contextmenu() {}
		static control*		createbyurl(const char* url);
		color				getcolor(color normal) const;
		virtual char*		getdescription(char* result) const;
		virtual char*		getname(char* result) const;
		virtual const widget* gettoolbar() const { return 0; }
		virtual const char*	geturl(char* result) const { return 0; }
		virtual bool		ismodified() const { return false; }
		virtual bool		keyinput(int id);
		virtual void		nonclient(rect rc);
		int					open(const char* title, int state = 0, int width = 600, int height = 400); // Open in new window and make modal loop
		bool				open(rect rc);
		virtual void		prerender() {}
		virtual void		redraw(rect rc) {}
	};
	xsfield					control_type[];
	struct form : control
	{
		widget::proc		type;
		bool				need_close;
		form();
		command*			getcommands() const override;
		xsfield*			getmeta() const override { return control_type; }
		int					render(int x, int y, int width, widget* childs);
	};
	namespace dialog
	{
		int					application(const char* title, unsigned flags = -1, bool allow_multiply_window = false);
		control*			find(const char* url);
		control*			getworking();
		void				initialize();
		control*			openbyurl(const char* url, bool make_active = true);
		void				remove(draw::control* object);
		int					select(draw::control** objects);
	}
	xscontext::command		form_commands[];
	typedef char*			(*proctext)(char* result, void* object);
	bool					buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key = 0, bool press = false, const char* tooltips_text = 0);
	bool					buttonv(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key = 0, bool press = false);
	void					dockbar(rect& rc);
	void					execute(int cid, int param, element& e);
	unsigned				getdocked(control** output, unsigned count, dock_types type);
	void					hilight(rect rc, bool focused);
	void					scrollh(const char* id, const rect& scroll, int& origin, int count, int maximum, bool focused);
	void					scrollv(const char* id, const rect& scroll, int& origin, int count, int maximum, bool focused);
	int						sheetline(rect rc, bool background = true);
	void					splith(int x, int y, int width, int& value, const char* id, int size, int minimum, int maximum, bool down_align);
	void					splitv(int x, int y, int& value, int height, const char* id, int size, int minimum, int maximum, bool right_align);
	int						tabs(rect rc, bool show_close, bool right_side, void** data, int start, int count, int current, int* hilite, draw::proctext gtext, draw::proctext gstate = 0, rect position = {2, 2, -2, -2});
	bool					tool(rect rc, bool disabled, bool checked, bool press);
	int						view(widget::proc p, int x, int y, int width, const char* id, const char* label, unsigned flags = 0);
	void					view(rect rc, control& e, bool show_toolbar = true);
	int						view(rect rc, control** pages, int count, int& current, bool show_toolbar, unsigned tab_state, int padding);
}
extern draw::element		current_widget_element;
extern xsfield				widget_type[];
int wdt_button(int x, int y, int width, draw::element& e);
int wdt_check(int x, int y, int width, draw::element& e);
int wdt_clipart(int x, int y, int width, draw::element& e);
int wdt_field(int x, int y, int width, draw::element& e);
int	wdt_group(int x, int y, int width, draw::element& e);
int wdt_horizontal(int x, int y, int width, draw::element& e);
int wdt_label(int x, int y, int width, draw::element& e);
int wdt_radio(int x, int y, int width, draw::element& e);
int wdt_separator(int x, int y, int width, draw::element& e);
int wdt_statusbar();
int wdt_tabs(int x, int y, int width, draw::element& e);
int wdt_tool(int x, int y, int width, draw::element& e);
int wdt_toolbar(int x, int y, int width, draw::element& e);
int wdt_title(int x, int y, int width, draw::element& e);
int	wdt_vertical(int x, int y, int width, draw::element& e);
int wdt_vertical(int x, int y, int width, draw::element& e, int* source, int count, int start);