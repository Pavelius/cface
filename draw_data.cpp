#include "draw.h"
#include "xsref.h"

using namespace draw;

static context*		hot_source;

int	draw::getdata(context* source, const char* id)
{
	if(!source)
		return 0;
	xsref e = {source->getmeta(), source};
	return e.get(id);
}

static void callback_setvalue()
{
	if(!hot_source)
		return;
	xsref e = {hot_source->getmeta(), hot_source};
	e.set(hot::name, hot::param);
}

void draw::setdata(context* source, const char* id, int value)
{
	execute(InputSetValue, value);
	hot::name = id;
	hot_source = source;
	hot::callback = callback_setvalue;
}