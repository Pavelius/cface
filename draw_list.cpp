#include "crt.h"
#include "draw.h"
#include "draw_list.h"
#include "xsref.h"

using namespace draw;
using namespace draw::controls;

list::list() : origin(0), maximum(0), current(0),
maximum_width(0), origin_width(0),
lines_per_page(0), pixels_per_line(0),
show_grid_lines(false),  hilite_rows(false)
{
	id = "list";
}

void list::ensurevisible()
{
	if(current<origin)
		origin = current;
	if(current>origin + lines_per_page - 1)
		origin = current - lines_per_page + 1;
}

bool list::isopen(int row)
{
	return (row<maximum - 1) ? (getlevel(row + 1)>getlevel(row)) : false;
}

void list::toggle(int index)
{
	if(!isgroup(index))
		return;
	int level = getlevel(index);
	int mm = maximum;
	int cc = current;
	if(isopen(index))
		collapse(index);
	else
		expand(index, level);
	prerender();
	if(cc>index)
	{
		if(mm<maximum)
			current += maximum-mm;
	}
}

void list::correction()
{
	if(current>=maximum)
		current = maximum-1;
	if(current<0)
		current = 0;
	if(lines_per_page)
	{
		if(origin>maximum-lines_per_page)
			origin = maximum-lines_per_page;
		if(origin<0)
			origin = 0;
	}
}

void list::row(rect rc, int index)
{
	if(index==current)
		hilight(rc, focused);
}

int list::getroot(int row) const
{
	while(true)
	{
		auto parent = getparent(row);
		if(parent == -1)
			return row;
		row = parent;
	}
}

int list::getparent(int row) const
{
	int level = getlevel(row);
	while(row)
	{
		if(level > getlevel(row))
			return row;
		row--;
	}
	if(level > getlevel(row))
		return row;
	return -1;
}

int list::getlastchild(int i) const
{
	int level = getlevel(i);
	int i2 = maximum-1;
	while(i<i2)
	{
		if(level!=getlevel(i+1))
			break;
		i++;
	}
	return i;
}

void list::background(rect& rc)
{
	control::background(rc);
	if(!pixels_per_line)
		pixels_per_line = texth() + 8;
}

void list::redraw(rect rc)
{
	if(!pixels_per_line)
		return;
	rect scroll = {0};
	rect scrollh = {0};
	lines_per_page = rc.height()/pixels_per_line;
	correction();
	if(maximum>lines_per_page)
		scroll.set(rc.x2-metrics::scroll, rc.y1, rc.x2, rc.y2);
	if(maximum_width>rc.width())
		scrollh.set(rc.x1, rc.y2-metrics::scroll, rc.x2, rc.y2);
	// mouse input handle
	if(hilite_rows && hot::key==MouseMove)
		current = -1;
	int rk = hot::key&CommandMask;
	if(draw::areb(rc))
	{
		// Обработаем выбор мышкой
		if(hot::pressed && (rk==MouseLeft || rk==MouseRight))
		{
			if(hot::mouse.y>rc.y1 && hot::mouse.y<=rc.y1+pixels_per_line*(maximum-origin))
			{
				if(!scroll.width() || hot::mouse.x<scroll.x1)
				{
					int curn = origin + (hot::mouse.y - rc.y1 - 2)/pixels_per_line;
					rect rc1 = {rc.x1, rc.y1 + 2 + (curn-origin)*pixels_per_line, rc.x2, rc.y1 + 2 + (curn-origin+1)*pixels_per_line};
					if(selecting(rc1, curn, hot::mouse))
						current = curn;
					if(maximum>lines_per_page)
						scroll.set(rc.x2-metrics::scroll, rc.y1, rc.x2, rc.y2);
					else
						scroll.clear();
				}
			}
		}
		// Если надо подсвечивать
		if(hilite_rows
			&& (rk == MouseMove || rk == MouseWheelDown || rk == MouseWheelUp || rk == MouseLeft || rk == MouseRight || rk == MouseLeftDBL))
		{
			if(!scroll.width() || hot::mouse.x<scroll.x1)
				current = origin + (hot::mouse.y - rc.y1) / pixels_per_line;
		}
	}
	if(true)
	{
		draw::state push;
		setclip(rc);
		int x1 = rc.x1;
		int y1 = rc.y1;
		int rw = rc.x2 - x1 + 1;
		int ix = origin;
		while(true)
		{
			if(y1>=rc.y2)
				break;
			if(ix>=maximum)
				break;
			rect rcm = {x1-origin_width, y1, rc.x1 + rw, y1+pixels_per_line};
			if(show_grid_lines)
				line(rcm.x1, rcm.y2-1, rcm.x2, rcm.y2-1, colors::form);
			row(rcm, ix);
			y1 += pixels_per_line;
			ix++;
		}
	}
	if(scroll)
		draw::scrollv(id, scroll, origin, lines_per_page, maximum, focused);
	if(scrollh)
		draw::scrollh(id, scrollh, origin_width, rc.width(), maximum_width, focused);
}

static unsigned execute_keyup(control* context, bool run)
{
	auto pc = (list*)context;
	pc->current--;
	pc->correction();
	pc->ensurevisible();
	return Executed;
}

static unsigned execute_keydown(control* context, bool run)
{
	auto pc = (list*)context;
	pc->current++;
	pc->correction();
	pc->ensurevisible();
	return Executed;
}

static unsigned execute_home(control* context, bool run)
{
	auto pc = (list*)context;
	if(pc->current == 0)
		return Disabled;
	pc->current = 0;
	pc->correction();
	pc->ensurevisible();
	return Executed;
}

static unsigned execute_end(control* context, bool run)
{
	auto pc = (list*)context;
	pc->current = pc->maximum - 1;
	pc->correction();
	pc->ensurevisible();
	return Executed;
}

static unsigned execute_page_up(control* context, bool run)
{
	auto pc = (list*)context;
	if(pc->current != pc->origin)
		pc->current = pc->origin;
	else
		pc->current -= pc->lines_per_page - 1;
	pc->correction();
	pc->ensurevisible();
	return Executed;
}

static unsigned execute_page_down(control* context, bool run)
{
	auto pc = (list*)context;
	if(pc->current != (pc->origin + pc->lines_per_page - 1))
		pc->current = (pc->origin + pc->lines_per_page - 1);
	else
		pc->current += pc->lines_per_page - 1;
	pc->correction();
	pc->ensurevisible();
	return Executed;
}

static unsigned execute_mouse_dbl_click(control* context, bool run)
{
	auto pc = (list*)context;
	if(!hot::mouse.in(hot::element))
		return Disabled;
	pc->invoke("change");
	return Executed;
}

static unsigned execute_mouse_wheel_up(control* context, bool run)
{
	auto pc = (list*)context;
	pc->origin--;
	if(pc->origin<0)
		pc->origin = 0;
	return Executed;
}

static unsigned execute_mouse_wheel_down(control* context, bool run)
{
	auto pc = (list*)context;
	pc->origin++;
	if(pc->origin>pc->maximum - pc->lines_per_page)
		pc->origin = pc->maximum - pc->lines_per_page;
	if(pc->origin<0)
		pc->origin = 0;
	return Executed;
}

control::command list_commands[] = {
	{"keyup", "Вверх", execute_keyup, 0, {KeyUp}, 0, HideCommand},
	{"keydown", "Вниз", execute_keydown, 0, {KeyDown}, 0, HideCommand},
	{0}
};

listfilter::listfilter() :filter(0)
{
}

listdata::listdata(const void** source, unsigned count, const xsfield* fields, const char* name) : source(source), fields(fields), name(name)
{
	maximum = count;
}

void listdata::row(rect rc, int index)
{
	list::row({rc.x1, rc.y1, rc.x2 - 1, rc.y2}, index);
	xsref e = {fields, (void*)source[index]};
	auto p = (const char*)e.get(name);
	if(p)
	{
		rc.offset(4);
		text(rc.x1, rc.y1, p);
	}
}