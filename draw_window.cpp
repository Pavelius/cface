#include "aref.h"
#include "crt.h"
#include "io.h"
#include "draw.h"

struct window_persistent
{
	const char*		id;
	rect			position;
};
static aref<window_persistent>	windows;

static struct windows_persistent_strategy : public io::strategy
{

	void write(io::writer& file, void* param) override
	{
		for(auto& e : windows)
		{
			file.open(e.id);
			file.set("x1", e.position.x1);
			file.set("y1", e.position.y1);
			file.set("x2", e.position.x2);
			file.set("y2", e.position.y2);
			file.close(e.id);
		}
	}

	void set(io::node& n, int value)
	{
		if(!n.parent)
			return;
		auto e = windows.find(szdup(n.parent->name));
		if(!e)
		{
			windows.reserve();
			e = &windows.add();
			memset(e, 0, sizeof(windows.data[0]));
			e->id = szdup(n.parent->name);
		}
		if(strcmp(n.name, "x1") == 0)
			e->position.x1 = value;
		else if(strcmp(n.name, "y1") == 0)
			e->position.y1 = value;
		else if(strcmp(n.name, "x2") == 0)
			e->position.x2 = value;
		else if(strcmp(n.name, "y2") == 0)
			e->position.y2 = value;
	}

	windows_persistent_strategy() : strategy("windows", "settings") {}

} windows_persistent_strategy_instance;

void draw::window::resizing(const rect& rc)
{
	if(draw::canvas)
	{
		draw::canvas->resize(rc.x2, rc.y2, draw::canvas->bpp, true);
		draw::clipping.set(0, 0, rc.x2, rc.y2);
	}
}

void draw::window::opening()
{
	focus = getfocus();
	if(!identifier)
		return;
	auto e = windows.find(szdup(identifier));
	if(!e)
		return;
	position = e->position;
}

void draw::window::closing()
{
	setfocus(focus, true);
	if(!identifier)
		return;
	auto e = windows.addu(szdup(identifier));
	if(!e)
		return;
	e->id = szdup(identifier);
	e->position = position;
}