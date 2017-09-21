#include "crt.h"
#include "draw.h"
#include "xsref.h"

using namespace draw;

static wrapper*		hot_source;

static void callback_setvalue()
{
	if(!hot_source)
		return;
	xsref e = {hot_source->getmeta(), hot_source};
	e.set(hot::name, hot::param);
}

int	draw::getdata(wrapper* source, const char* id)
{
	if(!source)
		return 0;
	xsref e = {source->getmeta(), source->getobject()};
	return e.get(id);
}

void draw::setdata(wrapper* source, const char* id, int value)
{
	execute(InputSetValue, value);
	hot::name = id;
	hot_source = source;
	hot::callback = callback_setvalue;
}

char* draw::getdata(char* temp, wrapper* source, const char* id, const draw::widget* childs, bool to_buffer)
{
	auto field = source->getmeta()->find(id);
	if(!field)
		return 0;
	auto value = getdata(source, id);
	if(field->type == text_type)
	{
		if(to_buffer)
			zcpy(temp, (char*)value);
		return (char*)value;
	}
	temp[0] = 0;
	if(field->type == number_type)
	{
		if(childs)
		{
			for(auto p = childs; *p; p++)
			{
				if(p->value == value && p->label)
				{
					zcpy(temp, p->label);
					return temp;
				}
			}
		}
		szprint(temp, "%1i", value);
	}
	return temp;
}