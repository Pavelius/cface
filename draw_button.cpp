#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "widget.h"

using namespace draw;

int wdt_clipart(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, control* source, int title, const widget* childs, const char* tips)
{
	if(label[0] != ':')
		return 0;
	auto p = label + 1;
	if(strcmp(p, "check") == 0)
	{
		rect rc = {x, y, x + 14, y + 14};
		rectf(rc, colors::window);
		color b = getcolor(colors::border, flags);
		line(rc.x1, rc.y1, rc.x2, rc.y1, b);
		line(rc.x2, rc.y1, rc.x2, rc.y2, b);
		line(rc.x2, rc.y2, rc.x1, rc.y2, b);
		line(rc.x1, rc.y2, rc.x1, rc.y1, b);
		if(flags&Checked)
		{
			state push;
			fore = getcolor(rc, colors::text, colors::text.mix(colors::active, 32), flags);
			linw = 1.3f;
			line(x + 3, y + 6, x + 6, y + 10);
			line(x + 6, y + 10, x + 11, y + 3);
		}
		return rc.height();
	}
	else if(strcmp(p, "radio") == 0)
	{
		rect rc = {x, y, x + 16, y + 16};
		circlef(x + 7, y + 6, 6, colors::window);
		circle(x + 7, y + 6, 6, getcolor(colors::border, flags));
		if(flags&Checked)
		{
			color b = getcolor(rc, colors::text, colors::text.mix(colors::active, 32), flags);
			circlef(x + 7, y + 6, 2, b);
			circle(x + 7, y + 6, 2, b);
		}
		return rc.height();
	}
	else if(strcmp(p, "dropdown") == 0)
	{
		static point clipart_dropdown[] =
		{
			{-2, -1}, {-1, -1}, {0, -1}, {1, -1}, {2, -1},
			{-1, 0}, {0, 0}, {1, 0},
			{0, 1}
		};
		state push;
		rect rc = {x, y, x + width, y + 16};
		x += width / 2;
		y += 12;
		fore = getcolor(rc, colors::black, colors::blue, flags);
		for(auto e : clipart_dropdown)
			pixel(x + e.x, y + e.y);
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
			wdt_clipart(rc.x1, rc.y1, rc.width(), "clipart", 0, string, 0, 0, 0, 0, 0, 0);
		}
		else
			text(rc, string, AlignCenter);
	}
	return result;
}

bool draw::buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key, bool press, const char* tips)
{
	state push;
	bool result = false;
	struct rect rcb = {rc.x1 + 1, rc.y1 + 1, rc.x2, rc.y2};
	areas a = area(rc);
	if(disabled)
	{
		gradv(rcb, colors::button.lighten(), colors::button.darken());
		if(border)
			rectb(rc, colors::border.mix(colors::window));
		fore = fore.mix(colors::button, 64);
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
			gradv(rcb, a1, a2);
			break;
		case AreaHilitedPressed:
			gradv(rcb, a2, a1);
			break;
		default:
			gradv(rcb, b1, b2);
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
			wdt_clipart(rc.x1, rc.y1, rc.width(), "clipart", flags, string, 0, 0, 0, 0, 0, 0);
		}
		else
			text(rc, string, AlignCenterCenter);
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