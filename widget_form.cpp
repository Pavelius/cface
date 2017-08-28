#include "crt.h"
#include "command.h"
#include "draw.h"
#include "xsref.h"
#include "widget.h"

using namespace draw;

int wdt_group(int x, int y, int width, draw::element& e)
{
	assert(e.childs);
	int y0 = y;
	e.setposition(x, y, width); // Первая рамка (может надо двойную ?)
	int x1 = x, y1 = y, w1 = width;
	e.setposition(x, y, width); // Отступ от рамки
	draw::state push;
	draw::font = metrics::font;
	if(e.label)
		y += draw::texth() + metrics::padding * 2;
	if(e.childs[0].width)
		y += wdt_horizontal(x, y, width, e);
	else
		y += wdt_vertical(x, y, width, e);
	if(e.label)
	{
		color c1 = colors::border.mix(colors::window, 128);
		color c2 = c1.darken();
		draw::gradv({x1, y1, x1 + w1, y1 + draw::texth() + metrics::padding * 2}, c1, c2);
		draw::fore = colors::text.mix(c1, 96);
		draw::text(x1 + (w1 - draw::textw(e.label)) / 2, y1 + metrics::padding, e.label);
	}
	y += metrics::padding;
	draw::rectb({x1, y1, x1 + w1, y}, colors::border);
	return y - y0;
}

int wdt_label(int x, int y, int width, draw::element& e)
{
	assert(e.label);
	draw::state push;
	e.setposition(x, y, width);
	e.decortext();
	return draw::textf(x, y, width, e.label) + metrics::padding * 2;
}

unsigned execute_close(xscontext* context, bool run)
{
	if(run)
		((form*)context)->need_close = 1;
	return Executed;
}

form::form() : need_close(false)
{
	id = "form";
	type = wdt_vertical;
}

xscontext::command draw::form_commands[] = {
	{"close", "Закрыть", execute_close},
	{0}
};

xscontext::command* form::getcommands() const
{
	return form_commands;
}

int form::render(int x, int y, int width, widget* childs)
{
	if(!type)
		return 0;
	draw::element e;
	e.type = type;
	e.context = this;
	e.childs = childs;
	return type(x, y, width, e);
}