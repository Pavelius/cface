#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "bsdata.h"
#include "widget.h"

using namespace draw;

static struct dock_info
{
	int			current;
	int			size;
	bool		visible;
} dock_data[DockWorkspace + 1];

static char* get_control_name(char* result, void* p)
{
	result[0] = 0;
	return ((control*)p)->getname(result);
}

unsigned draw::getdocked(control** output, unsigned count, dock_s type)
{
	auto ps = output;
	auto pe = output + count;
	for(auto p = control::plugin::first; p; p = p->next)
	{
		if(p->element.disabled)
			continue;
		if(p->element.dock == type)
		{
			if(ps < pe)
				*ps++ = &p->element;
		}
	}
	*ps = 0;
	return ps - output;
}

// view control on form
int	draw::view(rect rc, control** pages, int count, int& current, bool show_toolbar, unsigned tab_state, int padding)
{
	int y1 = rc.y1;
	if(current >= count)
		current = count - 1;
	if(padding == -1)
		padding = metrics::padding;
	auto& ec = *pages[current];
	if(count > 1)
	{
		int current_hilite = -1;
		const int dy = texth() + 8;
		line(rc.x1, rc.y1 + dy - 1, rc.x2, rc.y1 + dy, colors::border);
		rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
		if(tabs(rct, false, false, (void**)pages, 0, count,
			current, &current_hilite, get_control_name))
		{
			if(current_hilite != -1)
				current = current_hilite;
			else
				current = 0;
		}
		rc.y1 += dy;
	}
	ec.viewf(rc, show_toolbar);
	return rc.y1 - y1;
}

static bool dock_paint(dock_s id, rect& client, draw::control** p1, int c1, draw::control** p2, int c2)
{
	bool show_toolbar = true;
	rect rc = client;
	dock_info& e1 = dock_data[id - DockLeft];
	dock_info& e2 = dock_data[id - DockLeft + 1];
	if(!c1 && !c2)
		return false;
	if(!e1.size)
		e1.size = 200;
	if(!e2.size)
		e2.size = 200;
	const int sx = metrics::padding + 2;
	switch(id)
	{
	case DockLeft:
		draw::splitv(rc.x1, rc.y1, e1.size, rc.height(), (int)&dock_data[id], sx, 64, 400, false);
		client.x1 += e1.size + sx;
		rc.x2 = rc.x1 + e1.size;
		break;
	case DockRight:
		draw::splitv(rc.x2, rc.y1, e1.size, rc.height(), (int)&dock_data[id], sx, 64, 400, true);
		client.x2 -= e1.size + sx;
		rc.x1 = rc.x2 - e1.size;
		break;
	case DockBottom:
		draw::splith(rc.x1, rc.y2, rc.width(), e1.size, (int)&dock_data[id], sx + 1, 64, 400, true);
		client.y2 -= e1.size + sx + 1;
		rc.y1 = rc.y2 - e1.size;
		break;
	default:
		return false;
	}
	unsigned tab_state = NoBackground;
	if(!c2)
		draw::view(rc, p1, c1, e1.current, show_toolbar, tab_state, 0);
	else if(!c1)
		draw::view(rc, p2, c2, e2.current, show_toolbar, tab_state, 0);
	else if(id == DockLeft || id == DockRight)
	{
		draw::splith(rc.x1, rc.y1, rc.width(), e2.size, (int)&dock_data[id], sx, 64, 400, false);
		draw::view({rc.x1, rc.y1, rc.x2, rc.y1 + e2.size}, p1, c1, e1.current,
			show_toolbar, tab_state, 0);
		draw::view({rc.x1, rc.y1 + e2.size + sx, rc.x2, rc.y2}, p2, c2, e2.current,
			show_toolbar, tab_state, 0);
	}
	return true;
}

void draw::dockbar(rect& rc)
{
	const unsigned max_controls = 65;
	control* p1[max_controls + 1];
	control* p2[max_controls + 1];
	if(metrics::show::left)
		dock_paint(DockLeft, rc, p1, getdocked(p1, max_controls, DockLeft), p2, getdocked(p2, max_controls, DockLeftBottom));
	if(metrics::show::right)
		dock_paint(DockRight, rc, p1, getdocked(p1, max_controls, DockRight), p2, getdocked(p2, max_controls, DockRightBottom));
	if(metrics::show::bottom)
		dock_paint(DockBottom, rc, p1, getdocked(p1, max_controls, DockBottom), 0, 0);
}