#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "widget.h"

using namespace		draw;

extern rect			sys_static_area;
sprite*				metrics::toolbar = (sprite*)loadb("toolbar.pma");
sprite*				metrics::tree = (sprite*)loadb("tree.pma");
control::plugin*	control::plugin::first;

control::plugin::plugin(control& element) : element(element)
{
	seqlink(this);
}

char* control::getname(char* result) const
{
	if(id)
		zcpy(result, id);
	return result;
}

char* control::getdescription(char* result) const
{
	return result;
}

color control::getcolor(color normal) const
{
	if(disabled)
		return normal.mix(colors::window);
	return normal;
}

void control::background(rect& rc)
{
	if(!disabled && show_background)
		draw::rectf(rc, colors::window);
	if(show_border)
		draw::rectb(rc, getcolor(colors::border));
}

void control::nonclient(rect rc)
{
	redraw(rc);
}

bool control::open(const char* title, unsigned state, int width, int height)
{
	draw::window dc(-1, -1, width, height, WFMinmax | WFResize);
	if(title)
		draw::setcaption(title);
	setfocus(0);
	while(true)
	{
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form);
		rc.offset(metrics::padding*2);
		view(rc);
		int id = draw::input();
		if(id == KeyEscape)
			return false;
		if(focused)
			keyinput(id);
	}
}

bool control::open(rect rc)
{
	sys_static_area.set(0, 0, draw::getwidth(), draw::getheight());
	setfocus(id);
	while(true)
	{
		view(rc);
		int id = draw::input();
		switch(id)
		{
		case KeyEscape:
		case InputUpdate:
		case 0:
			return false;
		case MouseLeft:
			if(hot::pressed)
			{
				if(!area(rc))
					return false;
				draw::execute(MouseLeftDBL);
				continue;
			}
			break;
		case KeyEnter:
		case MouseLeftDBL:
			return true;
		}
		if(focused)
			keyinput(id);
	}
}

static void invoke_context_menu()
{
	((control*)hot::source)->contextmenu();
}

void control::view(rect rc, bool show_toolbar)
{
	draw::state push;
	struct rect rt = {rc.x1, rc.y1, rc.x2, rc.y1};
	auto commands = getcommands();
	if(show_toolbar && metrics::toolbar && commands && this->show_toolbar)
	{
		rt.y2 += metrics::toolbar->get(0).sy + 4;
		rc.y1 += rt.height() + metrics::padding;
	}
	// Common mouse and keyboard event
	unsigned flags = 0;
	if(disabled)
		flags |= Disabled;
	draw::focusing(id, rc, flags);
	focused = isfocused(flags);
	background(rc);
	// ������ �� ����� ������� ��������
	if(areb(rc))
	{
		hot::element = rc;
		if(!disabled)
		{
			if(!hot::pressed && hot::key == MouseRight)
				draw::execute(invoke_context_menu);
		}
	}
	// ����� ������� �������� ������ ��������.
	prerender();
	// ������� ���� ��������, ������� ����� ��������
	// ��������� 'background' ����� �������� ����� ��������.
	// ������� ������ ������� ������ ��� ����� ���������� ��������.
	nonclient(rc);
	if(rt.height())
		render(rt.x1, rt.y1, rt.width(), commands);
}

static void execute_tab(wrapper* object, bool run)
{
	auto pc = (control*)object;
	setfocus(getnext(getfocus(), KeyTab));
}

void control::keyinput(int id)
{
	auto pc = getcommands();
	if(!pc)
		return;
	auto p = pc->find(id);
	if(p)
		p->type(this, true);
}

//int wdt_separator(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
//{
//	auto height = metrics::toolbar->height;
//	//if(e.separator && *e.separator)
//	//	return 0;
//	rectf({x + 2, y + 2, x + 3, y + height - 2}, colors::border);
//	//if(e.separator)
//	//	*e.separator = true;
//	return height;
//}

static void invoke_callback()
{
	hot::source->execute(hot::name, true);
}

void control::invoke(const char* name) const
{
	draw::execute(invoke_callback);
	hot::source = (wrapper*)this;
	hot::name = name;
}

int control::render(int x, int y, int width, unsigned flags, const wrapper::command& e) const
{
	rect rc = {x, y, x + width, y + width};
	if(tool(rc, isdisabled(flags), false, true))
		invoke(e.id);
	switch(e.view)
	{
	case ViewIcon:
		image(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2,
			metrics::toolbar, e.icon, 0,
			(isdisabled(flags)) ? 0x80 : 0xFF);
		break;
	}
	if(areb(rc))
	{
		auto name = e.label;
		if(name)
		{
			if(e.key[0])
			{
				char temp[128];
				tooltips("%1 (%2)", name, key2str(temp, e.key[0]));
			}
			else
				tooltips(name);
		}
		statusbar("��������� ������� '%1'", name);
	}
	return width;
}

int	control::render(int x, int y, int width, const wrapper::command* commands) const
{
	if(!commands)
		return 0;
	if(!metrics::toolbar)
		return 0;
	int x2 = x + width - 6;
	auto height = metrics::toolbar->get(0).getrect(0,0,0).height() + 4;
	for(auto p = commands; *p; p++)
	{
		if(!p->type)
			continue;
		if(p->view == HideCommand)
			continue;
		auto width = height;
		if(x + width > x2)
		{
			// wdt_dropdown(x, y, 6, "toolbar_dropdown", 0, 0, 0, 0, source, 0, p, 0);
			break;
		}
		render(x, y, width, p->type((wrapper*)this, false), *p);
		x += width;
	}
	return height + metrics::padding * 2;
}

int wdt_control(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
{
	if(!source)
		return 0;
	auto pc = source->getwrapper(draw::getdatasource(id, link));
	if(!pc)
		return 0;
	if(!value)
		value = draw::getheight() - y;
	setposition(x, y, width);
	value -= metrics::padding*2;
	auto pcz = (control*)pc;
	pcz->view({x, y, x + width, y + value}, true);
	return value;
}

WIDGET(control);