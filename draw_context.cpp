#include "crt.h"
#include "draw.h"

using namespace draw;

const context::command* context::command::find(const char* id) const
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

const context::command* context::command::find(int id) const
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
		else if(p->key[0]==id || p->key[1]==id)
			return p;
		p++;
	}
	return 0;
}

unsigned context::execute(const char* id, bool run)
{
	auto p = getcommands()->find(id);
	if(p)
		return p->type(this, run);
	return 0;
}