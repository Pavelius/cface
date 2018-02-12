#include "collections.h"
#include "crt.h"
#include "io_plugin.h"
#include "draw.h"

struct window_persistent {
	const char* id;
	rect position;
};
static arefc<window_persistent> windows;

static window_persistent* find_window(const char* id) {
	for(auto& e : windows) {
		if(e.id == id)
			return &e;
	}
	return 0;
}

static window_persistent* add_window(const char* id) {
	auto p = find_window(id);
	if(p)
		return p;
	p = windows.add();
	memset(p, 0, sizeof(*p));
	p->id = id;
	return p;
}

static struct windows_persistent_strategy : public io::strategy {

	void write(io::writer& file, void* param) override {
		for(auto& e : windows) {
			file.open(e.id);
			file.set("x1", e.position.x1);
			file.set("y1", e.position.y1);
			file.set("x2", e.position.x2);
			file.set("y2", e.position.y2);
			file.close(e.id);
		}
	}

	void set(io::reader::node& n, const char* value) {
		if(!n.parent)
			return;
		auto e = add_window(szdup(n.parent->name));
		if(strcmp(n.name, "x1") == 0)
			e->position.x1 = sz2num(value);
		else if(strcmp(n.name, "y1") == 0)
			e->position.y1 = sz2num(value);
		else if(strcmp(n.name, "x2") == 0)
			e->position.x2 = sz2num(value);
		else if(strcmp(n.name, "y2") == 0)
			e->position.y2 = sz2num(value);
	}

	windows_persistent_strategy() : strategy("windows", "settings") {}

} windows_persistent_strategy_instance;

void draw::window::resizing(const rect& rc) {
	if(draw::canvas) {
		draw::canvas->resize(rc.x2, rc.y2, draw::canvas->bpp, true);
		draw::clipping.set(0, 0, rc.x2, rc.y2);
	}
}

void draw::window::opening() {
	focus = getfocus();
	if(!identifier)
		return;
	auto e = find_window(szdup(identifier));
	if(!e)
		return;
	position = e->position;
}

void draw::window::closing() {
	setfocus(focus, true);
	if(!identifier)
		return;
	auto e = add_window(szdup(identifier));
	e->position = position;
}