#include "point.h"
#include "surface.h"
#include "sprite.h"
#include "wrapper.h"

#pragma once
#define WIDGET(n) static widget::plugin plugin_##n(#n, wdt_##n)

extern "C" void* memset(void* destination, int value, unsigned size);
struct xsfield;

enum draw_event_s {
	// input events
	InputUser = 0xE000,
	InputSymbol = 0xED00, InputTimer,
	InputKeyUp, InputIdle, InputUpdate, InputNoUpdate,
	// widgets events
	InputChoose, InputDropDown, InputSetFocus,
	InputExecute, InputMenu, InputSetValue, InputEdit,
	ScrollV, ScrollH, SplitterV, SplitterH,
	TreeType, TreeFlags,
	HtmlLink, HtmlControl, TabsControl, TabsCloseControl,
	Executed,
	// Keyboard and mouse input (can be overrided by flags)
	MouseLeft = 0xEE00, MouseLeftDBL, MouseRight,
	MouseMove, MouseWheelUp, MouseWheelDown,
	KeyLeft, KeyRight, KeyUp, KeyDown, KeyPageUp, KeyPageDown, KeyHome, KeyEnd,
	KeyBackspace, KeyEnter, KeyDelete, KeyEscape, KeySpace, KeyTab,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	// named keys range
	Alpha,
	FirstKey = MouseLeft,
	FirstMouse = MouseLeft, LastMouse = MouseWheelDown,
	// support
	CommandMask = 0x0000FFFF,
	// misc events can be combination with previous
	Ctrl = 0x00010000,
	Alt = 0x00020000,
	Shift = 0x00040000,
	// control visual flags
	NoBorder = 0x01000000,
	NoBackground = 0x02000000,
	NoToolbar = 0x04000000,
	NoFocus = 0x08000000,
	// state flags
	Focused = 0x10000000, // Control has keyboard input and can change visual form.
	Checked = 0x20000000, // Use in background virtual method.
	Disabled = 0x40000000, // Control not grant any input.
	FirstInput = InputSymbol,
};
enum window_flags {
	WFResize = 0x0010,
	WFMinmax = 0x0020,
	WFMaximized = 0x0040,
	WFAbsolutePos = 0x0080,
};
enum cursors {
	CursorArrow, CursorHand, CursorLeftRight, CursorUpDown, CursorAll, CursorNo, CursorEdit, CursorWait,
};
enum areas {
	AreaNormal, // Area not have mouse
	AreaHilited, // Area have mouse
	AreaHilitedPressed, // Area have mouse and mouse button is pressed
};
enum dock_s {
	DockLeft, DockLeftBottom,
	DockRight, DockRightBottom,
	DockBottom, DockWorkspace,
};
enum iflags
{
	ImageMirrorV = 0x0001,
	ImageMirrorH = 0x0002,
	ImageGrayscale = 0x0004,
	ImageNoOffset = 0x0008,
	ImageTransparent = 0x0010,
	ImageColor = 0x0020,
	ImagePallette = 0x0040,
	TextStroke = 0x0080,
	TextItalic = 0x0100,
	TextBold = 0x0200,
	TextUscope = 0x0400,
	TextSingleLine = 0x0800, // Text would be showed as single line
	AlignLeft = 0x0000,
	AlignCenter = 0x1000,
	AlignRight = 0x2000,
	AlignLeftCenter = 0x3000,
	AlignCenterCenter = 0x4000,
	AlignRightCenter = 0x5000,
	AlignLeftBottom = 0x6000,
	AlignCenterBottom = 0x7000,
	AlignRightBottom = 0x8000,
	AlignWidth = 0xE000,
	AlignMask = 0xF000,
};
typedef char* (*proctext)(char* result, void* object);
namespace hot
{
	typedef void			(*proc)(); // Hot callback reaction
	extern int				animate; // Frame tick count
	extern cursors			cursor; // set this mouse cursor
	extern int				key; // [in] if pressed key or mouse this field has key
	extern point			mouse; // current mouse coordinates
	extern bool				pressed; // flag if any of mouse keys is pressed
	extern int				param; // Draw command context. Application can extend this structure
	extern const char*		name; // Text name of element (optional)
	extern rect				element; // Element coordinates
	extern proc				callback; // Callback proc
	extern wrapper*			source; // Data source wrapper
	extern bool				stop; // Set true if we need stop modal loop
}
namespace colors
{
	extern color			active;
	extern color			button;
	extern color			form;
	extern color			window;
	extern color			border;
	extern color			text, edit, h1, h2, h3, special;
	namespace tips
	{
		extern color		back;
		extern color		text;
	}
	namespace tabs
	{
		extern color		back;
		extern color		text;
	}
}
namespace metrics
{
	extern rect				edit;
	extern sprite*			font;
	extern sprite*			h1;
	extern sprite*			h2;
	extern sprite*			h3;
	extern sprite*			icons;
	extern sprite*			toolbar;
	extern sprite*			tree;
	extern int				h3s;
	extern int				padding;
	extern int				scroll;
	namespace show
	{
		extern bool			left;
		extern bool			right;
		extern bool			bottom;
		extern bool			padding;
		extern bool			statusbar;
	}
}
namespace draw
{
	enum field_type_s {FieldText, FieldNumber, FieldReference};
	namespace drag
	{
		extern const char*	id;
		extern int			part;
		extern int			value;
		extern point		mouse;
		bool				active(const char* id, int part = 0);
		void				begin(const char* id, int part = 0);
	}
	namespace clipboard
	{
		void				copy(const void* string, int lenght);
		int					paste(void* data, int maxlenght = 0);
	}
	namespace dialog
	{
		bool				color(struct color& result, struct color* custom = 0);
		bool				folder(const char* title, char* path);
		bool				open(const char* title, char* path, const char* filter, int filter_index = 0, const char* ext = 0);
		bool				save(const char* title, char* path, const char* filter, int filter_index = 0, const char* ext = 0);
	}
	struct state // Push state in stack
	{
		state();
		~state();
	private:
		unsigned char		forc;
		color				fore;
		float				linw;
		const sprite*		font; // glyph font
		color*				palt;
		surface*			canvas;
		rect				clip;
		bool				mouseinput;
	};
	struct widget
	{
		// Each widget draw by this procedure
		typedef int(*proc)(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips);
		// Plugin for widget descriptor
		struct plugin
		{
			const char*		id;
			proc			render;
			plugin*			next;
			static plugin*	first;
			plugin(const char* id, proc render);
			static plugin*	find(const char* id);
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
		operator bool() const { return type != 0; }
	};
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
		control();
		//
		virtual void		background(rect& rc);
		virtual void		contextmenu() {}
		color				getcolor(color normal) const;
		virtual char*		getdescription(char* result) const;
		virtual char*		getname(char* result) const;
		virtual const widget* gettoolbar() const { return 0; }
		virtual bool		keyinput(int id);
		virtual void		nonclient(rect rc);
		bool				open(rect rc);
		bool				open(const char* title, unsigned state, int width, int height);
		virtual void		redraw(rect rc) {}
		virtual void		prerender() {}
		void				view(rect rc, bool show_toolbar = false);
	};
	// Output system window
	struct window : surface, state
	{
		window*				parent;
		unsigned			flags;
		const char*			identifier; // Identifier for storing and restoring positions
		point				minimum;
		rect				position;
		int					focus;
		void*				hwnd;  // Platform specific handles
		void*				gc; // Platform specific handles
		//
		window(int x, int y, int width, int height, unsigned flags, int bpp = 32, const char* identifier = 0);
		~window();
		//
		void				closing();
		void				opening();
		void				resizing(const rect& rc);
	};
	extern rect				clipping; // Clipping area
	extern color			fore; // Foreground color (curently selected color)
	extern const sprite*	font; // Currently selected font
	//
	bool					addbutton(rect& rc, const char* label, int key, const char* tips);
	int						addbutton(rect& rc, const char* t1, int k1, const char* tt1, const char* t2, int k2, const char* tt2);
	void					addelement(const char* id, const rect& rc);
	int						aligned(int x, int width, unsigned state, int string_width);
	int						alignedh(const rect& rc, const char* string, unsigned state);
	areas					area(rect rc);
	bool					areb(rect rc);
	void					bezier(int x0, int y0, int x1, int y1, int x2, int y2);
	void					bezierseg(int x0, int y0, int x1, int y1, int x2, int y2);
	void					blit(surface& dest, int x, int y, int width, int height, unsigned flags, surface& dc, int xs, int ys);
	void					blit(surface& dest, int x, int y, int width, int height, unsigned flags, surface& source, int x_source, int y_source, int width_source, int height_source);
	bool					buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key = 0, bool press = false, const char* tips = 0);
	bool					buttonv(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key = 0, bool press = false);
	extern surface*			canvas;
	void					circle(int x, int y, int radius);
	void					circle(int x, int y, int radius, const color c1);
	void					circlef(int x, int y, int radius, const color c1, unsigned char alpha = 0xFF);
	void					decortext(unsigned flags);
	void					dockbar(rect& rc);
	bool					domodal();
	void					execute(int id, int value = 0);
	void					focusing(const char* id, const rect& rc, unsigned& flags);
	int						getbpp();
	color					getcolor(color normal, unsigned flags);
	color					getcolor(rect rc, color normal, color hilite, unsigned flags);
	int						getdata(wrapper* source, const char* id);
	char*					getdata(char* temp, wrapper* source, const char* id, const draw::widget* childs, bool to_buffer, field_type_s& type);
	inline const char*		getdatasource(const char* id, const char* link) { return link ? link : id; }
	unsigned				getdocked(control** output, unsigned count, dock_s type);
	const char*				getfocus();
	int						getheight();
	const char*				getnext(const char* id, int key);
	int						getwidth();
	window*					getwindow();
	void					glyph(int x, int y, int sym, unsigned flags);
	void					gradv(rect rc, const color c1, const color c2, int skip = 0);
	void					gradh(rect rc, const color c1, const color c2, int skip = 0);
	void					hilight(rect rc, unsigned flags);
	int						hittest(int x, int test_x, const char* string, int lenght);
	int						hittest(rect rc, const char* string, unsigned state, point mouse);
	bool					hittest(int x, int y, const sprite* e, int id, int flags, point mouse);
	inline bool				ischecked(unsigned flags) { return (flags&Checked) != 0; }
	inline bool				isdisabled(unsigned flags) { return (flags&Disabled) != 0; }
	inline bool				isfocused(unsigned flags) { return (flags&Focused) != 0; }
	void					image(int x, int y, const sprite* e, int id, int flags, unsigned char alpha = 0xFF);
	void					image(int x, int y, const sprite* e, int id, int flags, unsigned char alpha, color* pal);
	void					initialize();
	int						input(bool redraw = false);
	void					line(int x1, int y1, int x2, int y2); // Draw line
	void					line(int x1, int y1, int x2, int y2, unsigned char color); // Draw line
	void					line(int x1, int y1, int x2, int y2, color c1); // Draw line
	inline void				line(point p1, point p2, color c1) { line(p1.x, p1.y, p2.x, p2.y, c1); }
	void					linet(int x1, int y1, int x2, int y2);
	extern float			linw;
	extern char				link[2048];
	extern bool				mouseinput;
	extern color*			palt;
	void					pixel(int x, int y);
	void					pixel(int x, int y, unsigned char alpha);
	unsigned char*			ptr(int x, int y);
	int						rawinput();
	void					rectb(rect rc); // Draw rectangle border
	void					rectb(rect rc, unsigned char c1);
	void					rectb(rect rc, color c1);
	void					rectf(rect rc); // Draw rectangle area. Right and bottom side is one pixel less.
	void					rectf(rect rc, unsigned char c1); // Draw rectangle area. Right and bottom side is one pixel less.
	void					rectf(rect rc, color c1);
	void					rectf(rect rc, color c1, unsigned char alpha);
	void					rectf(rect rc, unsigned char c1, unsigned char alpha);
	void					rectx(rect rc, color c1);
	void					scrollh(const char* id, const struct rect& scroll, int& origin, int count, int maximum, bool focused = false);
	void					scrollv(const char* id, const struct rect& scroll, int& origin, int count, int maximum, bool focused = false);
	void					setcaption(const char* string);
	void					setclip(rect rc);
	inline void				setclip() { clipping.set(0, 0, getwidth(), getheight()); }
	void					setcolor(unsigned char index);
	void					setdata(wrapper* source, const char* id, int value, bool instant = false);
	void					setfocus(const char* id);
	void					setposition(int& x, int& y, int& width);
	void					settimer(unsigned milleseconds);
	int						sheetline(rect rc, bool background = true);
	const char*				skiptr(const char* string);
	void					spline(point* points, int n);
	void					splith(int x, int y, int width, int& value, const char* id, int size, int minimum, int maximum, bool down_align = false);
	void					splitv(int x, int y, int& value, int height, const char* id, int size, int minimum, int maximum, bool right_align = false);
	void					stroke(int x, int y, const sprite* e, int id, int flags, unsigned char thin = 1, unsigned char* koeff = 0);
	void					syscursor(bool enable);
	void					sysmouse(bool enable);
	void					sysredraw();
	int						tabs(rect rc, bool show_close, bool right_side, void** data, int start, int count, int current, int* hilite, proctext gtext, proctext gstate = 0, rect position = {0,0,0,0});
	void					text(int x, int y, const char* string, int count = -1, unsigned flags = 0);
	int						text(rect rc, const char* string, unsigned state = 0, int* max_width = 0);
	int						textc(int x, int y, int width, const char* string, int count = -1, unsigned flags = 0);
	int						textbc(const char* string, int width);
	int						textlb(const char* string, int index, int width, int* line_index = 0, int* line_count = 0);
	int						texte(rect rc, const char* string, unsigned flags, int i1, int i2);
	int						textf(int x, int y, int width, const char* text, int* max_width = 0, int min_height = 0, int* cashe_height = 0, const char** cashe_string = 0, int tab_width = 0);
	int						textf(rect& rc, const char* string, int tab_width = 0);
	int						texth();
	int						texth(const char* string, int width);
	int						textw(int sym);
	int						textw(const char* string, int count = -1);
	int						textw(rect& rc, const char* string);
	int						textw(sprite* font);
	bool					tool(rect rc, bool disabled, bool checked, bool press);
	int						view(rect rc, control** pages, int count, int& current, bool show_toolbar, unsigned tab_state, int padding);
}
int							isqrt(int num);
char*						key2str(char* result, int key);
void						statusbar(const char* format, ...);
void						set_light_theme();
void						set_dark_theme();
void						tooltips(const char* format, ...);
void						tooltips(int x, int y, const char* format, ...);
void						tooltips(rect rc, const char* format, ...);

int wdt_button(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_check(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_clipart(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_field(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_group(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_label(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_radio(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_radio_element(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_separator(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_tabs(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_title(int& x, int y, int& width, unsigned flags, const char* label, int title);
int wdt_tool(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_toolbar(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_vertical(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_horizontal(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, wrapper* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);