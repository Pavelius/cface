#include "crt.h"
#include "draw.h"
#include "xsref.h"
#include "widget.h"

using namespace draw;

static void callback_setvalue(wrapper* source, const char* id, int value)
{
	if(!source)
		return;
	xsref e = {source->getmeta(), source};
	e.set(id, value);
}

static void callback_setvalue()
{
	callback_setvalue(hot::source, hot::name, hot::param);
}

int	draw::getdata(wrapper* source, const char* id)
{
	if(!source)
		return 0;
	xsref e = {source->getmeta(), source->getobject()};
	return e.get(id);
}

void draw::setdata(wrapper* source, const char* id, int value, bool instant)
{
	if(instant)
		callback_setvalue(source, id, value);
	else
	{
		execute(InputSetValue, value);
		hot::name = id;
		hot::source = source;
		hot::callback = callback_setvalue;
	}
}

char* draw::getdata(char* temp, wrapper* source, const char* id, const widget* childs, bool to_buffer, field_type_s& type)
{
	type = FieldNumber;
	auto field = source->getmeta()->find(id);
	if(!field)
		return 0;
	auto value = getdata(source, id);
	temp[0] = 0;
	if(field->type == text_type)
	{
		type = FieldText;
		if(to_buffer)
		{
			if(value)
				zcpy(temp, (char*)value);
			return temp;
		}
		if(!value)
			return "";
		return (char*)value;
	}
	if(field->type == number_type)
	{
		type = FieldNumber;
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
	else
	{
		type = FieldReference;
		xsref xr = {field->type, (void*)value};
		auto pv = xr.get("name");
		if(!pv)
			return "";
		if(!to_buffer)
			return (char*)pv;
		zcpy(temp, (char*)pv);
	}
	return temp;
}