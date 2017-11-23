#pragma once

#define WIDGET(n) static widget::plugin plugin_##n(#n, wdt_##n)

namespace draw
{
	struct control;
	struct widget
	{
		typedef int(*proc)(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, draw::control* source, int title, const widget* childs, const char* tips);
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
		widget&				settips(const char* v) { tips = v; return *this; }
		widget&				setvalue(int v) { value = v; return *this; }
	};
	int						getdata(control* source, const char* id);
	char*					getdata(char* temp, control* source, const char* id, const widget* childs, bool to_buffer, field_type_s& type);
	inline const char*		getdatasource(const char* id, const char* link) { return link ? link : id; }
	void					setdata(control* source, const char* id, int value, bool instant = false);
}
int wdt_button(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_check(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_control(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, draw::control* source, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_clipart(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_field(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_group(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_label(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_radio(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_radio_element(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_separator(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_tabs(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_title(int& x, int y, int& width, unsigned flags, const char* label, int title);
int wdt_tool(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_toolbar(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_vertical(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int wdt_horizontal(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);