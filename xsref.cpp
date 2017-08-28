#include "xsref.h"

extern "C" void*	memset(void* p1, unsigned char value, unsigned size);

int xsref::get(const char* id) const
{
	auto pf = fields->find(id);
	if(!pf)
		return 0;
	auto p = pf->ptr(object);
	return pf->get(p);
}

int xsref::getdf(const char* id, int default_value) const
{
	auto r = get(id);
	return r ? r : default_value;
}

const char* xsref::getdf(const char* id, const char* default_value) const
{
	auto r = (const char*)get(id);
	return r && r[0]? r : default_value;
}

int xsref::get(const char* id, unsigned index) const
{
	auto pf = fields->find(id);
	if(!pf || index>= pf->count)
		return 0;
	auto p = pf->ptr(object, index);
	return pf->get(p);
}

void xsref::set(const char* name, int value, unsigned index)
{
	if(!fields)
		return;
	auto pf = fields->find(name);
	if(!pf)
		return;
	const void* p;
	if(index)
	{
		if(index >= pf->count)
			return;
		p = pf->ptr(object, index);
	}
	else
		p = pf->ptr(object);
	if(((unsigned)p) <= 0xFFFF)
		return;
	pf->set(p, value);
}

xsref xsref::getr(const char* name, unsigned index) const
{
	if(!fields)
		return{0};
	auto pf = fields->find(name);
	if(!pf || index >= pf->count)
		return{0};
	if(pf->reference==0)
		return{pf->type, (void*)pf->ptr(object, index)};
	void* p = *((void**)pf->ptr(object, index));
	if(!p)
		return{0};
	return{pf->type, p};
}

xsref xsref::getvalue(const char* name, unsigned index) const
{
	if(!fields)
		return{0};
	auto pf = fields->find(name);
	if(!pf || index >= pf->count)
		return{0};
	return{pf, (void*)pf->ptr(object, index)};
}

const void* xsref::ptr(const char* name, unsigned index) const
{
	auto pf = fields->find(name);
	if(!pf || index >= pf->count)
		return 0;
	return pf->ptr(object, index);
}

void xsref::clear() const
{
	if(!fields)
		return;
	for(auto f = fields; f->id; f++)
		memset((void*)f->ptr(object), 0, f->lenght);
}

bool xsref::isempthy() const
{
	for(auto f = fields; f->id; f++)
	{
		auto p1 = f->ptr(object);
		auto p2 = p1 + f->lenght;
		while(p1 < p2)
		{
			if(*p1)
				return false;
		}
	}
	return true;
}