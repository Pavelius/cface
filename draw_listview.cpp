#include "crt.h"
#include "draw.h"
#include "draw_list.h"
#include "xsfield.h"

using namespace draw::controls;

listview::listview(const void** source, unsigned count, const xsfield* fields, const char* name) :
	source(source), fields(fields)
{
	maximum = count;
	setpresetation(name);
}

const char* listview::getname(int index) const
{
	return (const char*)requisit->get(requisit->ptr(source[index]));
}

int	listview::find(const char* name) const {
	for(auto i = 0; i < maximum; i++) {
		auto p = getname(i);
		if(p && strcmp(p, name) == 0)
			return i;
	}
	return -1;
}

void listview::setpresetation(const char* name)
{
	requisit = fields->find(name);
}

void listview::row(rect rc, int index)
{
	list::row({rc.x1+1, rc.y1+1, rc.x2-1, rc.y2}, index);
	draw::area(rc);
	auto p = (const char*)requisit->get(requisit->ptr(source[index]));
	if(p)
	{
		rc.offset(4, 4);
		text(rc.x1, rc.y1, p);
	}
}