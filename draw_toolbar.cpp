#include "draw.h"

using namespace draw;

bool draw::tool(rect rc, bool disabled, bool checked, bool press)
{
	if(disabled)
		return false;
	bool result = false;
	rect rch = rc; rch.x2--; rch.y2--;
	areas a = area(rch);
	if((a == AreaHilited || a == AreaHilitedPressed) && hot::key == MouseLeft && hot::pressed == press)
		result = true;
	if(checked)
		a = AreaHilitedPressed;
	switch(a)
	{
	case AreaHilited:
		rectf(rc, colors::edit.mix(colors::window, 128));
		rectb(rc, colors::border);
		break;
	case AreaHilitedPressed:
		rectf(rc, colors::edit);
		rectb(rc, colors::border);
		break;
	default:
		break;
	}
	return result;
}

int wdt_separator(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const draw::widget* childs, const char* tips)
{
	auto height = metrics::toolbar->height;
	//if(e.separator && *e.separator)
	//	return 0;
	rectf({x + 2, y + 2, x + 3, y + height - 2}, colors::border);
	//if(e.separator)
	//	*e.separator = true;
	return height;
}

static int wdt_dropdown(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, void* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0)
{
	auto height = metrics::toolbar->height;
	rect rc = {x, y, x + width, y + height};
	bool result = draw::tool(rc, false, false, true);
	wdt_clipart(x, y, width, id, flags, ":dropdown", 0, 0, 0, 0, childs);
	if(result)
	{
		draw::execute(InputDropDown);
	}
	return height;
}

int wdt_tool(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const draw::widget* childs, const char* tips)
{
	auto height = metrics::toolbar->height;
	rect rc = {x, y, x + height, y + height};
	if(tool(rc, isdisabled(flags), false, true))
	{
		draw::execute(InputExecute);
		hot::name = id;
	}
	image(rc.x1 + rc.width()/2, rc.y1 + rc.height() / 2,
		metrics::toolbar, value, 0,
		(isdisabled(flags)) ? 0x80 : 0xFF);
	if(areb(rc))
	{
		auto name = label;
		if(name)
			tooltips(name);
		if(tips)
			statusbar(tips);
		else
			statusbar("Выполнить команду '%1'", name);
	}
	return height;
}

int wdt_toolbar(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, draw::context* source, int title, const draw::widget* childs, const char* tips)
{
	if(!childs)
		return 0;
	if(!metrics::toolbar)
		return 0;
	int x2 = x + width - 6;
	auto height = metrics::toolbar->get(0).sy + 4;
	for(auto p = childs; *p; p++)
	{
		auto width = height;
		if(x + width > x2)
		{
			wdt_dropdown(x, y, 6, "toolbar_dropdown", 0, 0, 0, 0, source, 0, p);
			break;
		}
		p->type(x, y, width, p->id, p->flags, p->label, p->value, p->link, source, p->title, p->childs, p->tips);
		x += width;
	}
	return height + metrics::padding * 2;
}