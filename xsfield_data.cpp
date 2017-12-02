#include "crt.h"
#include "xsbase.h"

const char* xsfield::getdata(char* result, const char* id, const void* object, bool tobuffer) const
{
	result[0] = 0;
	auto requisit = find(id);
	if(!requisit)
		return 0;
	if(requisit->type == text_type)
	{
		auto value = (const char*)requisit->get(requisit->ptr(object));
		if(value)
		{
			if(!tobuffer)
				return value;
			zcpy(result, value);
		}
	}
	else if(requisit->type == number_type)
	{
		auto value = requisit->get(requisit->ptr(object));
		sznum(result, value);
	}
	else if(requisit->reference)
	{
		object = (void*)requisit->get(requisit->ptr((void*)object));
		if(!object)
			return result;
		auto value_type = requisit->type;
		if(value_type->type == text_type)
			requisit = value_type;
		else
			requisit = value_type->find("name");
		if(!requisit)
			return result;
		auto value = (const char*)requisit->get(requisit->ptr(object));
		if(value)
		{
			if(!tobuffer)
				return value;
			zcpy(result, value);
		}
	}
	return result;
}

void xsfield::setdata(const char* result, const char* id, void* object) const
{
	auto requisit = find(id);
	if(!requisit)
		return;
	if(requisit->type == text_type)
	{
		if(result[0] == 0)
			result = 0;
		else
			result = szdup(result);
		requisit->set(requisit->ptr(object), (int)result);
	}
	else if(requisit->type == number_type)
		requisit->set(requisit->ptr(object), sz2num(result));
	else if(requisit->reference)
	{
		if(result[0]==0)
			requisit->set(requisit->ptr(object), 0);
		else
		{
			auto xs = xsbase::getref(result, requisit->type);
			if(xs)
				requisit->set(requisit->ptr(object), (int)xs.object);
		}
	}
}