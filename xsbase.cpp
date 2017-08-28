#include "crt.h"
#include "xsbase.h"

xsbase* xsbase::first;

xsbase::xsbase(const char* id, void* data, unsigned size, unsigned lenght, const xsfield* fields, const void* clone) :
	amem(size), id(id), fields(fields), clone(clone)
{
	this->data = data;
	this->size = size;
	if(size)
		this->count_maximum = lenght / size;
	this->count = this->count_maximum;
	seqlink(this);
	for(unsigned i = 0; i < count_maximum; i++)
	{
		auto e = getref(i);
		if(e && !e.get(fields->id))
		{
			count = i;
			break;
		}
	}
}

xsref xsbase::find(const char* id, const char* value)
{
	xsref e1 = {fields, data};
	auto ps = (const char**)e1.ptr(id);
	if(ps)
	{
		for(unsigned i = 0; i < count; i++)
		{
			if(*ps && strcmp(*ps, value) == 0)
				return{fields, (void*)get(i)};
			ps = (const char**)((char*)ps + size);
		}
	}
	return{0, 0};
}

xsref xsbase::getref(const char* id)
{
	for(auto xs = first; xs; xs = xs->next)
	{
		if(!xs->fields->iskey())
			continue;
		auto xr = xs->find("id", id);
		if(xr)
			return xr;
	}
	return{0};
}

xsref xsbase::getrefbyptr(const void* value)
{
	if(!value)
		return{0};
	for(auto xs = first; xs; xs = xs->next)
	{
		auto index = xs->indexof(value);
		if(index==-1)
			continue;
		return{xs->fields, (void*)xs->get(index)};
	}
	return{0};
}

xsref xsbase::find(const char* base, const char* id, const char* value)
{
	auto xs = find(base);
	if(!xs)
		return{0, 0};
	return xs->find(id, value);
}

xsref xsbase::getref(int index) const
{
	if(((unsigned)index) >= ((unsigned)count))
		return {0};
	return{fields, (void*)get(index)};
}

void* xsbase::add(const void* object)
{
	if(count >= count_maximum)
		return 0;
	auto p = (void*)get(count++);
	if(object)
		memcpy(p, object, size);
	return p;
}

xsbase* xsbase::find(const char* id)
{
	if(!id || !id[0])
		return 0;
	for(auto p = first; p; p = p->next)
	{
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

xsbase* xsbase::find(const xsfield* type)
{
	if(!type)
		return 0;
	for(auto p = first; p; p = p->next)
	{
		if(p->fields==type)
			return p;
	}
	return 0;
}

xsbase* xsbase::findbyptr(const void* object)
{
	if(!object)
		return 0;
	for(auto p = first; p; p = p->next)
	{
		if(object>=p->data && object<=(char*)p->data + p->count_maximum*p->size)
			return p;
	}
	return 0;
}

void xsbase::clear()
{
	if(clone)
	{
		for(unsigned i = 0; i < count; i++)
		{
			auto p = (void*)get(i);
			if(p)
				memcpy(p, clone, size);
		}
	}
	count = 0;
}