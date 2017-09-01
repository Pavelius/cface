#include "draw.h"
#include "widget.h"

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

int wdt_separator(int x, int y, int width, draw::element& e)
{
	if(e.separator && *e.separator)
		return 0;
	rectf({x + 2, y + 2, x + 3, y + e.height - 2}, colors::border);
	if(e.separator)
		*e.separator = true;
	e.width = 4;
	return e.height;
}

static int wdt_dropdown(int x, int y, int width, draw::element& e)
{
	rect rc = {x, y, x + width, y + e.height};
	bool result = draw::tool(rc, false, false, true);
	wdt_clipart(x, y, width, e);
	if(result)
	{
		e.rectangle = rc;
		draw::execute(InputDropDown, 0, e);
	}
	return e.height;
}

int wdt_tool(int x, int y, int width, draw::element& e)
{
	rect rc = {x, y, x + e.height, y + e.height};
	if(tool(rc, e.isdisabled(), false, true))
		draw::execute(InputExecute, 0, e);
	image(rc.x1 + rc.width()/2, rc.y1 + rc.height() / 2,
		metrics::toolbar, e.value, 0,
		(e.isdisabled()) ? 0x80 : 0xFF);
	if(areb(rc))
	{
		auto pc = e.context->getcommands()->find(e.id);
		auto name = e.label;
		if(!name && pc)
			name = pc->label;
		if(name)
			tooltips(name);
		if(e.tips)
			statusbar(e.tips);
		else
			statusbar("Выполнить команду '%1'", name);
	}
	return e.height;
}

int wdt_toolbar(int x, int y, int width, draw::element& e)
{
	if(!e.childs)
		return 0;
	if(!metrics::toolbar)
		return 0;
	int x2 = x + width - 6;
	e.height = metrics::toolbar->get(0).sy + 4;
	for(auto p = e.childs; *p; p++)
	{
		draw::element e1(*p, &e);
		e1.height = e.height;
		if(!e1.width)
			e1.width = e.height;
		if(x + e1.width > x2)
		{
			draw::element e2(*p, &e);
			e2.type = wdt_dropdown;
			wdt_dropdown(x, y, 6, e2);
			break;
		}
		e1.type(x, y, e.height, e1);
		x += e1.width;
	}
	return e.height + metrics::padding * 2;
}