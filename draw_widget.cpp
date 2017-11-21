#include "crt.h"
#include "draw.h"
#include "widget.h"

using namespace draw;

widget::plugin* widget::plugin::first;

widget::plugin::plugin(const char* id, proc render) : id(id), render(render)
{
	seqlink(this);
}

widget::plugin* widget::plugin::find(const char* id)
{
	for(auto p = first; p; p = p->next)
	{
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

static char* get_text(char* result, void* object)
{
	if(((widget*)object)->label)
		zcpy(result, ((widget*)object)->label, 259);
	return result;
}

int wdt_tabs(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
{
	if(!childs)
		return 0;
	auto y0 = y;
	const int tab_height = 24 + 4;
	y += 1;
	sheetline({x, y0, x + width, y + tab_height});
	const widget* data[32];
	auto ps = data;
	auto pe = data + sizeof(data) / sizeof(data[0]);
	for(auto p = childs; *p; p++)
	{
		if(ps < pe)
			*ps++ = p;
	}
	auto count = ps - data;
	int current = getdata(source, getdatasource(id, link));
	rect rc = {x, y, x + width, y + tab_height};
	int tabs_hilite;
	if(draw::tabs(rc, false, false, (void**)data, 0, count, current, &tabs_hilite, get_text))
	{
		if(tabs_hilite != -1)
			setdata(source, getdatasource(id, link), tabs_hilite);
	}
	y += tab_height + metrics::padding;
	auto& pw = childs[current];
	return pw.type(x, y, width, pw.id, pw.flags, pw.label, pw.value, pw.link, source, pw.title, pw.childs, pw.tips) + (y - y0);
}