#include "crt.h"
#include "draw.h"
#include "widget.h"

static point clipart_dropdown[] =
{
	{-2, -1}, {-1, -1}, {0, -1}, {1, -1}, {2, -1},
	{-1, 0}, {0, 0}, {1, 0},
	{0, 1}
};

int wdt_clipart(int x, int y, int width, draw::element& e)
{
	if(e.label[0] != ':')
		return 0;
	auto p = e.label + 1;
	if(strcmp(p, "check") == 0)
	{
		rect rc = {x, y, x + 14, y + 14};
		draw::rectf(rc, colors::window);
		color b = e.getcolor(colors::border);
		draw::line(rc.x1, rc.y1, rc.x2, rc.y1, b);
		draw::line(rc.x2, rc.y1, rc.x2, rc.y2, b);
		draw::line(rc.x2, rc.y2, rc.x1, rc.y2, b);
		draw::line(rc.x1, rc.y2, rc.x1, rc.y1, b);
		if(e.ischecked())
		{
			draw::state push;
			draw::fore = e.getcolor(rc, colors::text, colors::text.mix(colors::active, 32));
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
		draw::circle(x + 7, y + 6, 6, e.getcolor(colors::border));
		if(e.ischecked())
		{
			color b = e.getcolor(rc, colors::text, colors::text.mix(colors::active, 32));
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
		draw::fore = e.getcolor(rc, colors::black, colors::blue);
		for(auto e : clipart_dropdown)
			draw::pixel(x + e.x, y + e.y);
		return rc.height();
	}
	return 0;
}

bool draw::buttonv(int x, int y, int width, int height, bool checked, bool focused, bool disabled, bool border, const char* string, int key, bool press)
{
	bool result = false;
	struct rect rc = {x, y, x + width, y + height};
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
			draw::element e;
			e.label = string;
			if(disabled)
				e.flags |= Disabled;
			if(focused)
				e.flags |= Focused;
			wdt_clipart(rc.x1, rc.y1, rc.width(), e);
		}
		else
			text(rc, string, AlignCenter);
	}
	return result;
}

bool draw::buttonh(int x, int y, int width, int height, bool checked, bool focused, bool disabled, bool border, const char* string, int key, bool press, const char* tooltips_text)
{
	draw::state push;
	bool result = false;
	struct rect rc = {x, y, x + width, y + height};
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
			draw::element e;
			e.label = string;
			if(disabled)
				e.flags |= Disabled;
			if(focused)
				e.flags |= Focused;
			wdt_clipart(rc.x1, rc.y1, rc.width(), e);
		}
		else
			draw::text(rc, string, AlignCenterCenter);
	}
	if(tooltips_text && a == AreaHilited)
	{
		char temp[32];
		if(key)
			tooltips("%1 (%2)", tooltips_text, key2str(temp, key));
		else
			tooltips(tooltips_text);
	}
	return result;
}

int wdt_radio_element(int x, int y, int width, draw::element& e)
{
	assert(e.label);
	assert(e.parent);
	draw::state push;
	e.setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, e.label);
	rc1.offset(-2);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	e.focusing(rc);
	e.decortext();
	draw::element e1 = e;
	e1.label = ":radio";
	wdt_clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, e1);
	bool need_select = false;
	auto a = draw::area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && !e.isdisabled() && hot::key == MouseLeft)
	{
		if(!hot::pressed)
			need_select = true;
		else
			draw::execute(InputSetFocus, 0, e);
	}
	if(e.isfocused())
	{
		draw::rectx({rc1.x1, rc1.y1, rc1.x2, rc1.y2}, draw::fore);
		if(!e.isdisabled() && hot::key == KeySpace)
			need_select = true;
	}
	if(need_select)
	{
		auto pm = e.getparent(wdt_radio);
		if(pm)
			draw::execute(InputSetValue, e.value, *pm);
	}
	draw::text({rc1.x1 + 2, rc1.y1 + 2, rc1.x2 - 2, rc1.y2 - 2}, e.label);
	if(e.tips && a == AreaHilited)
		tooltips(e.tips);
	return rc1.height() + metrics::padding * 2;
}

int wdt_radio(int x, int y, int width, draw::element& e)
{
	assert(e.childs);
	int y0 = y;
	auto value = e.data.get();
	for(auto p = e.childs; *p; p++)
	{
		draw::element e1(*p, &e);
		if(p->value == value)
			e1.flags |= Checked;
		y += wdt_radio_element(x, y, width, e1);
	}
	return y - y0;
}

int wdt_check(int x, int y, int width, draw::element& e)
{
	assert(e.label);
	e.setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	if(e.data.get())
		e.flags |= Checked;
	draw::textw(rc1, e.label);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	e.focusing(rc);
	draw::element e1 = e;
	e1.label = ":check";
	wdt_clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, e1);
	e.decortext();
	auto a = draw::area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && !e.isdisabled() && hot::key == MouseLeft)
	{
		if(!hot::pressed)
			draw::execute(InputSetValue, e.ischecked() ? 0 : 1, e);
		else
			draw::execute(InputSetFocus, 0, e);
	}
	if(e.isfocused())
	{
		draw::rectx({rc1.x1 - 2, rc1.y1 - 1, rc1.x2 + 2, rc1.y2 + 1}, draw::fore);
		if(!e.isdisabled() && hot::key == KeySpace)
			draw::execute(InputSetValue, e.ischecked() ? 0 : 1, e);
	}
	draw::text(rc1, e.label);
	if(e.tips && a == AreaHilited)
		tooltips(e.tips);
	return rc1.height() + metrics::padding * 2;
}

int wdt_button(int x, int y, int width, draw::element& e)
{
	assert(e.label);
	e.setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()*e.getheight()};
	if(!e.isdisabled() && e.context)
	{
		auto result = e.context->execute(e.id, false);
		if(result == Disabled || !result)
			e.flags |= Disabled;
	}
	e.focusing(rc);
	if(draw::buttonh(x, y, width, rc.height(),
		e.ischecked(), e.isfocused(), e.isdisabled(), true,
		e.label, KeyEnter, false, e.tips))
		draw::execute(InputExecute, 0, e);
	return rc.height() + metrics::padding * 2;
}