#include "crt.h"
#include "wrapper.h"

wrapper*				hot::source;

const wrapper::command* wrapper::command::find(const char* id) const
{
	auto p = this;
	if(!p)
		return 0;
	while(*p)
	{
		if(p->child)
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

const wrapper::command* wrapper::command::find(int id) const
{
	auto p = this;
	if(!p)
		return 0;
	while(*p)
	{
		if(p->child)
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

unsigned wrapper::execute(const char* id, bool run)
{
	auto p = getcommands()->find(id);
	if(p)
		return p->type(this, run);
	return 0;
}

void wrapper::keyinput(int id)
{
	auto pc = getcommands();
	if(!pc)
		return;
	auto p = pc->find(id);
	if(p)
		p->type(this, true);
}