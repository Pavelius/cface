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
		const char* value = 0;
		if(requisit->size==sizeof(char) && requisit->count>1)
			value = (const char*)requisit->ptr(object);
		else
			value = (const char*)requisit->get(requisit->ptr(object));
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
	else
	{
		if(requisit->reference)
			object = (void*)requisit->get(requisit->ptr((void*)object));
		else if(requisit->size <= sizeof(int))
		{
			auto xs = xsbase::find(requisit->type);
			if(!xs)
				return result;
			auto index = requisit->get(requisit->ptr(object));
			object = xs->get(index);
		}
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
	else if(result[0]==0)
		requisit->set(requisit->ptr(object), 0);
	else
	{
		auto xs = xsbase::getref(result, requisit->type);
		if(xs)
		{
			if(requisit->reference)
				requisit->set(requisit->ptr(object), (int)xs.object);
			else
			{
				auto xb = xsbase::find(requisit->type);
				if(xb)
					requisit->set(requisit->ptr(object), xb->indexof(xs.object));
			}
		}
	}
}