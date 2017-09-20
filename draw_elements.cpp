#include "crt.h"
#include "draw.h"

using namespace draw;

int	draw::getdata(void* source, const char* id, const char* link)
{
	if(!source)
		return 0;
	return 0;
}

void draw::setdata(void* source, const char* id, const char* link, int value)
{
}

static void callback_setfocus()
{
	draw::setfocus(hot::name);
}

void draw::focusing(const char* id, const rect& rc, unsigned& flags)
{
	if(flags&Disabled)
		return;
	if(getfocus() == id)
		flags |= Focused;
	else if(area(rc) == AreaHilitedPressed && hot::key == MouseLeft && hot::pressed)
	{
		execute(InputSetFocus);
		hot::name = id;
		hot::callback = callback_setfocus;
	}
	addelement(id, rc);
}

int wdt_radio_element(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
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
	{
		draw::execute(InputSetValue, value);
		hot::name = id;
	}
	draw::text({rc1.x1 + 2, rc1.y1 + 2, rc1.x2 - 2, rc1.y2 - 2}, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + metrics::padding * 2;
}

int wdt_radio(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	int y0 = y;
	auto current_value = getdata(source, id, link);
	for(auto p = childs; *p; p++)
	{
		unsigned flags = 0;
		if(p->value == current_value)
			flags |= Checked;
		if(getfocus() == id)
			flags |= Focused;
		y += wdt_radio_element(x, y, width, id, flags, p->label, p->value);
	}
	return y - y0;
}

int wdt_check(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const draw::widget* childs, const char* tips)
{
	if(!label || !label[0])
		return 0;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	if(getdata(source, id, link))
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
		{
			draw::execute(InputSetFocus);
			hot::name = id;
		}
	}
	if(isfocused(flags))
	{
		draw::rectx({rc1.x1 - 2, rc1.y1 - 1, rc1.x2 + 2, rc1.y2 + 1}, draw::fore);
		if(!isdisabled(flags) && hot::key == KeySpace)
			need_value;
	}
	if(need_value)
	{
		draw::execute(InputSetValue, ischecked(flags) ? 0 : 1);
		hot::name = id;
	}
	draw::text(rc1, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + metrics::padding * 2;
}

int wdt_button(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, void* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0)
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
	}
	return rc.height() + metrics::padding * 2;
}

WIDGET(clipart);
WIDGET(button);
WIDGET(check);
WIDGET(radio);
WIDGET(radio_element);