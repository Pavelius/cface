#include "crt.h"
#include "draw.h"
#include "widget.h"

using namespace draw;

xsfield widget_type[] = {
	BSREQ(widget, id, text_type),
	BSREQ(widget, label, text_type),
	BSREQ(widget, childs, widget_type),
	BSREQ(widget, flags, number_type),
	BSREQ(widget, width, number_type),
	BSREQ(widget, height, number_type),
	BSREQ(widget, value, number_type),
	BSREQ(widget, title, number_type),
	BSREQ(widget, tips, number_type),
	BSREQ(widget, link, number_type),
	BSREQ(widget, maximum, number_type),
};

void draw::hilight(rect rc, bool focused)
{
	draw::rectf(rc, colors::edit);
	if(focused)
		draw::rectx({rc.x1, rc.y1, rc.x2-1, rc.y2-1}, colors::black);
}

void widget::clear()
{
	memset(this, 0, sizeof(widget));
}

void widget::decortext() const
{
	draw::fore = getcolor(colors::text);
}

bool widget::ischecked() const
{
	return (flags&Checked) != 0;
}

bool widget::isdisabled() const
{
	return (flags&Disabled) != 0;
}

bool widget::isfocused() const
{
	return (flags&Focused) != 0;
}

color widget::getcolor(color normal) const
{
	if(isdisabled())
		return normal.mix(colors::window);
	return normal;
}

color widget::getcolor(rect rc, color normal, color active) const
{
	if(isdisabled())
		return normal.mix(colors::window);
	if(areb(rc))
		return active;
	return normal;
}