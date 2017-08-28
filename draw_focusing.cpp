#include "command.h"
#include "crt.h"
#include "draw.h"

struct focusable_element
{
	const char*	id;
	rect		rc;
	operator bool() const { return id != 0; }	
};
static const char*			current_focus;
static focusable_element	elements[128];
static focusable_element*	render_control = elements;

COMMAND(clear_render)
{
	render_control = elements;
}

static focusable_element* getby(const char* id)
{
	if(!id)
		return 0;
	for(auto& e : elements)
	{
		if(!e)
			return 0;
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

static focusable_element* getfirst()
{
	for(auto& e : elements)
	{
		if(!e)
			return 0;
		return &e;
	}
	return 0;
}

static focusable_element* getlast()
{
	auto p = elements;
	for(auto& e : elements)
	{
		if(!e)
			break;
		p = &e;
	}
	return p;
}

void draw::addelement(const char* id, const rect& rc)
{
	if(!render_control
		|| render_control >= elements + sizeof(elements) / sizeof(elements[0]) - 1)
		render_control = elements;
	render_control[0].id = id;
	render_control[0].rc = rc;
	render_control[1].id = 0;
	render_control++;
}

const char* draw::getfocus()
{
	return current_focus;
}

void draw::setfocus(const char* id)
{
	current_focus = id;
}

const char* draw::getnext(const char* id, int key)
{
	if(!key)
		return id;
	auto pc = getby(id);
	if(!pc)
		pc = getfirst();
	if(!pc)
		return 0;
	auto pe = pc;
	auto pl = getlast();
	int inc = 1;
	if(key == KeyLeft || key == KeyUp || key == (KeyTab | Shift))
		inc = -1;
	while(true)
	{
		pc += inc;
		if(pc > pl)
			pc = elements;
		else if(pc < elements)
			pc = pl;
		if(pe == pc)
			return pe->id;
		switch(key)
		{
		case KeyRight:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 < pc->rc.x1)
				return pc->id;
			break;
		case KeyLeft:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 > pc->rc.x1)
				return pc->id;
			break;
		case KeyDown:
			if(pc->rc.y1 >= pe->rc.y2)
				return pc->id;
			break;
		case KeyUp:
			if(pc->rc.y2 <= pe->rc.y1)
				return pc->id;
			break;
		default:
			return pc->id;
		}
	}
}