#include "crt.h"
#include "xscontext.h"

xscontext::command* xscontext::command::find(const char* id)
{
	auto p = this;
	if(!p)
		return 0;
	while(*p)
	{
		if(!p->type)
		{
			auto v = p->child->find(id);
			if(v)
				return v;
		}
		else if(strcmp(p->id, id) == 0)
			return p;
		p++;
	}
	return 0;
}

xscontext::command* xscontext::command::findbykey(int id)
{
	auto p = this;
	if(!p)
		return 0;
	while(*p)
	{
		if(!p->type)
		{
			auto v = p->child->findbykey(id);
			if(v)
				return v;
		}
		else if(p->key[0]==id || p->key[1]==id)
			return p;
		p++;
	}
	return 0;
}

unsigned xscontext::execute(const char* id, bool run)
{
	auto p = getcommands()->find(id);
	if(p)
		return p->type(this, run);
	return 0;
}