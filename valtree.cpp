#include "agrw.h"
#include "valtree.h"

static agrw<valtree> objects;

static void cleanup(valtree* p)
{
	while(p)
	{
		auto m = p->next;
		if(p->type==valtree::Structure)
			cleanup((valtree*)p->value);
		memset(p, 0, sizeof(valtree));
		p = m;
	}
}

valtree::valtree() : name("root"), value(0), type(Number), next(0)
{
}

valtree::~valtree()
{
	cleanup(this);
}

void* valtree::operator new(unsigned size)
{
	for(auto ps = &objects; ps; ps = ps->next)
	{
		for(auto& e : *ps)
		{
			if(!e.name)
				return &e;
		}
	}
	return objects.add();
}

valtree& valtree::add(const char* name, valtree::types type, valtree* v)
{
	name = szdup(name);
	set(Structure);
	// Find existing
	auto p = (valtree*)find(name);
	if(p)
	{
		p->set(type);
		p->value = v;
		return *p;
	}
	// Add new one
	p = new valtree;
	p->name = name;
	p->type = type;
	p->value = v;
	if(value)
		seqlast(value)->next = p;
	else
		value = p;
	return *p;
}

const valtree* valtree::find(const char* name) const
{
	if(type != Structure)
		return 0;
	for(auto p = value; p; p = p->next)
	{
		if(strcmp(p->name, name)==0)
			return p;
	}
	return 0;
}

void valtree::set(valtree::types type)
{
	if(this->type == type)
		return;
	if(this->type == Structure && value)
		cleanup((valtree*)value);
	this->type = type;
	value = 0;
}

const char* valtree::gets(const char* name) const
{
	auto p = find(name);
	if(p && p->type == Text)
		return (char*)p->value;
	return "";
}

int valtree::geti(const char* name) const
{
	auto p = find(name);
	if(p && p->type == Number)
		return (int)p->value;
	return 0;
}