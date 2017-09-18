#include "crt.h"
#include "draw.h"
#include "widget.h"

using namespace draw;

static point clipart_dropdown[] =
{
	{-2, -1}, {-1, -1}, {0, -1}, {1, -1}, {2, -1},
	{-1, 0}, {0, 0}, {1, 0},
	{0, 1}
};

int wdt_clipart(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	if(label[0] != ':')
		return 0;
	auto p = label + 1;
	if(strcmp(p, "check") == 0)
	{
		rect rc = {x, y, x + 14, y + 14};
		draw::rectf(rc, colors::window);
		color b = getcolor(colors::border, flags);
		draw::line(rc.x1, rc.y1, rc.x2, rc.y1, b);
		draw::line(rc.x2, rc.y1, rc.x2, rc.y2, b);
		draw::line(rc.x2, rc.y2, rc.x1, rc.y2, b);
		draw::line(rc.x1, rc.y2, rc.x1, rc.y1, b);
		if(flags&Checked)
		{
			draw::state push;
			draw::fore = getcolor(rc, colors::text, colors::text.mix(colors::active, 32), flags);
			draw::linw = 1.3f;
			draw::line(x + 3, y + 6, x + 6, y + 10);
			draw::line(x + 6, y + 10, x + 11, y + 3);
		}
		return rc.height();
	}
	else if(strcmp(p, "radio") == 0)
	{
		rect rc = {x, y, x + 16, y + 16};
		draw::circlef(x + 7, y + 6, 6, colors::window);
		draw::circle(x + 7, y + 6, 6, getcolor(colors::border, flags));
		if(flags&Checked)
		{
			color b = getcolor(rc, colors::text, colors::text.mix(colors::active, 32), flags);
			draw::circlef(x + 7, y + 6, 2, b);
			draw::circle(x + 7, y + 6, 2, b);
		}
		return rc.height();
	}
	else if(strcmp(p, "dropdown") == 0)
	{
		draw::state push;
		rect rc = {x, y, x + width, y + 16};
		x += width / 2;
		y += 12;
		draw::fore = getcolor(rc, colors::black, colors::blue, flags);
		for(auto e : clipart_dropdown)
			draw::pixel(x + e.x, y + e.y);
		return rc.height();
	}
	return 0;
}

bool draw::buttonv(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key, bool press)
{
	bool result = false;
	if(disabled)
	{
		gradh(rc, colors::button.lighten(), colors::button.darken());
		rectf(rc, colors::border.mix(colors::window));
	}
	else
	{
		if(focused && key && hot::key == key)
			result = true;
		areas a = area(rc);
		if((a == AreaHilited || a == AreaHilitedPressed) && hot::key == MouseLeft && hot::pressed == press)
			result = true;
		if(checked)
			a = AreaHilitedPressed;
		color active = colors::button.mix(colors::edit, 128);
		switch(a)
		{
		case AreaHilited:
			gradh(rc, active.lighten(), active.darken());
			break;
		case AreaHilitedPressed:
			gradh(rc, active.darken(), active.lighten());
			break;
		default:
			gradh(rc, colors::button.lighten(), colors::button.darken());
			break;
		}
		if(border)
			rectb(rc, focused ? colors::active : colors::border);
	}
	if(string)
	{
		if(string[0] == ':')
		{
			unsigned flags = 0;
			if(disabled)
				flags |= Disabled;
			if(focused)
				flags |= Focused;
			wdt_clipart(rc.x1, rc.y1, rc.width(), "clipart", 0, string);
		}
		else
			text(rc, string, AlignCenter);
	}
	return result;
}

bool draw::buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key, bool press, const char* tips)
{
	draw::state push;
	bool result = false;
	struct rect rcb = {rc.x1 + 1, rc.y1 + 1, rc.x2, rc.y2};
	areas a = draw::area(rc);
	if(disabled)
	{
		draw::gradv(rcb, colors::button.lighten(), colors::button.darken());
		if(border)
			draw::rectb(rc, colors::border.mix(colors::window));
		draw::fore = draw::fore.mix(colors::button, 64);
	}
	else
	{
		if(focused && key && hot::key == key)
			result = true;
		if((a == AreaHilited || a == AreaHilitedPressed) && hot::key == MouseLeft && hot::pressed == press)
			result = true;
		if(checked)
			a = AreaHilitedPressed;
		color active = colors::button.mix(colors::edit, 128);
		color a1 = active.lighten();
		color a2 = active.darken();
		color b1 = colors::button.lighten();
		color b2 = colors::button.darken();
		switch(a)
		{
		case AreaHilited:
			draw::gradv(rcb, a1, a2);
			break;
		case AreaHilitedPressed:
			draw::gradv(rcb, a2, a1);
			break;
		default:
			draw::gradv(rcb, b1, b2);
			break;
		}
		if(border)
			draw::rectb(rc, focused ? colors::active : colors::border);
	}
	if(string)
	{
		if(string[0] == ':')
		{
			unsigned flags = 0;
			if(disabled)
				flags |= Disabled;
			if(focused)
				flags |= Focused;
			wdt_clipart(rc.x1, rc.y1, rc.width(), "clipart", flags, string);
		}
		else
			draw::text(rc, string, AlignCenterCenter);
	}
	if(tips && a == AreaHilited)
	{
		char temp[32];
		if(key)
			tooltips("%1 (%2)", tips, key2str(temp, key));
		else
			tooltips(tips);
	}
	return result;
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
	auto current_value = getdata(source, value);
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

int wdt_check(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	if(!label || !label[0])
		return 0;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	if(getdata(source, value))
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

int wdt_button(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, void* source = 0, int title = 0, const widget* childs = 0, const char* tips = 0)
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

WIDGET(button);
WIDGET(check);
WIDGET(radio);