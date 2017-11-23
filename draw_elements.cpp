#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "widget.h"

using namespace draw;

static void callback_setfocus()
{
	draw::setfocus(hot::name);
}

static void execute_setfocus(const char* id)
{
	execute(callback_setfocus);
	hot::name = id;
}

void draw::focusing(const char* id, const rect& rc, unsigned& flags)
{
	if(flags&Disabled)
		return;
	if(!getfocus())
		setfocus(id);
	if(getfocus() == id)
		flags |= Focused;
	else if(area(rc) == AreaHilitedPressed && hot::key == MouseLeft && hot::pressed)
		execute_setfocus(id);
	addelement(id, rc);
}

bool draw::addbutton(rect& rc, const char* t1, int k1, const char* tt1)
{
	const int width = 18;
	rc.x2 -= width;
	auto result = draw::buttonh({rc.x2, rc.y1, rc.x2 + width, rc.y2},
		false, false, false, false,
		t1, k1, true, tt1);
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
	return result;
}

int draw::addbutton(rect& rc, const char* t1, int k1, const char* tt1, const char* t2, int k2, const char* tt2)
{
	const int width = 20;
	rc.x2 -= width;
	auto height = rc.height() / 2;
	auto result = 0;
	if(draw::buttonh({rc.x2, rc.y1, rc.x2 + width, rc.y1 + height},
		false, false, false, false,
		t1, k1, true, tt1))
		result = 1;
	if(draw::buttonh({rc.x2, rc.y1 + height, rc.x2+width, rc.y1+height*2},
		false, false, false, false,
		t2, k2, true, tt2))
		result = 2;
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
	return result;
}

int wdt_radio_element(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, control* source, int title, const widget* childs, const char* tips)
{
	if(!label || !label[0])
		return 0;
	draw::state push;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc1.offset(-2);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	decortext(flags);
	focusing(id, rc, flags);
	wdt_clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, id, flags, ":radio");
	bool need_select = false;
	auto a = draw::area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot::key == MouseLeft)
	{
		if(!hot::pressed)
			need_select = true;
	}
	if(isfocused(flags))
	{
		draw::rectx({rc1.x1, rc1.y1, rc1.x2, rc1.y2}, draw::fore);
		if(!isdisabled(flags) && hot::key == KeySpace)
			need_select = true;
	}
	if(need_select)
		draw::setdata(source, link, value);
	draw::text({rc1.x1 + 2, rc1.y1 + 2, rc1.x2 - 2, rc1.y2 - 2}, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + metrics::padding * 2;
}

int wdt_radio(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, control* source, int title, const widget* childs, const char* tips)
{
	int y0 = y;
	auto current_value = getdata(source, getdatasource(id, link));
	for(auto p = childs; *p; p++)
	{
		unsigned flags = 0;
		if(p->value == current_value)
			flags |= Checked;
		if(getfocus() == id)
			flags |= Focused;
		y += wdt_radio_element(x, y, width, p->id, flags, p->label, p->value, getdatasource(id, link), source, title, 0, p->tips);
	}
	return y - y0;
}

int wdt_check(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, control* source, int title, const widget* childs, const char* tips)
{
	if(!label || !label[0])
		return 0;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	if(getdata(source, getdatasource(id, link)))
		flags |= Checked;
	draw::textw(rc1, label);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	focusing(id, rc, flags);
	wdt_clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, "clipart", flags, ":check");
	decortext(flags);
	auto a = draw::area(rc);
	auto need_value = false;
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot::key == MouseLeft)
	{
		if(!hot::pressed)
			need_value = true;
		else
			execute_setfocus(id);
	}
	if(isfocused(flags))
	{
		draw::rectx({rc1.x1 - 2, rc1.y1 - 1, rc1.x2 + 2, rc1.y2 + 1}, draw::fore);
		if(!isdisabled(flags) && hot::key == KeySpace)
			need_value;
	}
	if(need_value)
		draw::setdata(source, getdatasource(id, link), ischecked(flags) ? 0 : 1);
	draw::text(rc1, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + metrics::padding * 2;
}

int wdt_button(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, control* source, int title, const widget* childs, const char* tips)
{
	if(!label || !label[0])
		return 0;
	setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
	focusing(id, rc, flags);
	if(buttonh({x, y, x + width, rc.y2},
		ischecked(flags), isfocused(flags), isdisabled(flags), true,
		label, KeyEnter, false, tips))
	{
		draw::execute(InputExecute);
		hot::name = id;
		hot::source = source;
	}
	return rc.height() + metrics::padding * 2;
}

WIDGET(clipart);
WIDGET(button);
WIDGET(check);
WIDGET(radio);
WIDGET(radio_element);