#include "draw.h"

//void draw::hilight(rect rc, bool focused)
//{
//	const color c1 = focused ? colors::edit : colors::edit.mix(colors::window, 180);
//	rectf({rc.x1, rc.y1, rc.x2 - 1, rc.y2 - 1}, c1);
//	rectb({rc.x1, rc.y1, rc.x2 - 1, rc.y2 - 1}, c1);
//	if(focused)
//		rectx({rc.x1, rc.y1, rc.x2 - 1, rc.y2 - 1}, colors::text.mix(colors::form, 200));
//}

color draw::getcolor(color normal, unsigned flags)
{
	if(flags&Disabled)
		return normal.mix(colors::window);
	return normal;
}

color draw::getcolor(rect rc, color normal, color active, unsigned flags)
{
	if(flags&Disabled)
		return normal.mix(colors::window);
	if(areb(rc))
		return active;
	return normal;
}

void draw::decortext(unsigned flags)
{
	draw::fore = getcolor(colors::text, flags);
}

void draw::setposition(int& x, int& y, int& width)
{
	x += metrics::padding;
	y += metrics::padding;
	width -= metrics::padding * 2;
}

int	draw::getdata(void* source, int size)
{
	if(!source)
		return 0;
	switch(size)
	{
	case sizeof(int) : return *((int*)source);
	case sizeof(short) : return *((short*)source);
	case sizeof(char) : return *((char*)source);
	default: return 0;
	}
}

void draw::focusing(const char* id, const rect& rc, unsigned& flags)
{
	if(flags&Disabled)
		return;
	if(getfocus() == id)
		flags |= Focused;
	if(area(rc) == AreaHilitedPressed && hot::key == KeyLeft && hot::pressed)
	{
		execute(InputSetFocus);
		hot::name = id;
	}
	addelement(id, rc);
}