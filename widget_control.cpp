#include "crt.h"
#include "draw.h"
#include "widget.h"

extern rect			sys_static_area;
using namespace		draw;
sprite*				metrics::toolbar = (sprite*)loadb("toolbar.pma");
sprite*				metrics::tree = (sprite*)loadb("tree.pma");
control::plugin*	control::plugin::first;

xsfield	draw::control_type[] = {
	BSREQ(control, id, text_type),
	BSREQ(control, show_border, number_type),
	BSREQ(control, show_background, number_type),
	BSREQ(control, show_toolbar, number_type),
	BSREQ(control, disabled, number_type),
	BSREQ(control, focused, number_type),
	{0}
};

char* control::getname(char* result) const
{
	auto xr = getr();
	if(xr)
	{
		auto p = (const char*)xr.get("name");
		if(p)
			zcpy(result, p, 259);
	}
	return result;
}

char* control::getdescription(char* result) const
{
	auto xr = getr();
	if(xr)
	{
		auto p = (const char*)xr.get("text");
		if(p)
			zcpy(result, p, 259);
	}
	return result;
}

control::control() : id(0), show_border(true), show_background(true), disabled(false), focused(false)
{
}

control::plugin::plugin(control& value) : element(element)
{
	seqlink(this);
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

int control::open(const char* title, int state, int width, int height)
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
		draw::view(rc, *this);
		int id = draw::input();
		if(!id)
			return 0;
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
		draw::rectf(rc, colors::form);
		draw::view(rc, *this);
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

bool control::keyinput(int id)
{
	xscontext::command* pc;
	switch(id)
	{
	case InputMenu:
		contextmenu();
		break;
	case InputChoose:
		current_widget_element.choose();
		break;
	case InputEdit:
		current_widget_element.editing();
		break;
	case InputEditPlus:
		current_widget_element.addvalue(1);
		break;
	case InputEditMinus:
		current_widget_element.addvalue(-1);
		break;
	case KeyTab:
	case KeyTab | Shift:
		setfocus(getnext(getfocus(), id));
		break;
	case InputExecute:
		execute(current_widget_element.id, true);
		break;
	case InputSetFocus:
		draw::setfocus(current_widget_element.id);
		break;
	case InputSetValue:
		current_widget_element.data.set(hot::param);
		break;
	default:
		pc = getcommands()->findbykey(id);
		if(pc)
		{
			pc->type(this, true);
			break;
		}
		return false;
	}
	return true;
}

void draw::view(rect rc, control& co, bool show_toolbar)
{
	draw::state push;
	struct rect rt = {rc.x1, rc.y1, rc.x2, rc.y1};
	auto commands = co.gettoolbar();
	if(show_toolbar && metrics::toolbar && commands && co.show_toolbar)
	{
		rt.y2 += metrics::toolbar->get(0).sy + 4;
		rc.y1 += rt.height() + metrics::padding;
	}
	// Common mouse and keyboard event
	element ec(co); ec.focusing(rc);
	co.focused = ec.isfocused();
	co.background(rc);
	// Теперь мы имеем область элемента
	if(areb(rc))
	{
		hot::element = rc;
		if(!co.disabled)
		{
			if(!hot::pressed && hot::key == MouseRight)
				execute(InputMenu);
		}
	}
	// Перед выводом настроим разные элементы.
	co.prerender();
	// Обновим цвет элемента, который может именился
	// Процедура 'background' может изменить рамку элемента.
	// Поэтому только начиная отсюда она имеет корректное значение.
	co.nonclient(rc);
	if(rt.height())
	{
		draw::element e;
		e.parent = &ec;
		e.type = wdt_toolbar;
		e.childs = commands;
		e.context = &co;
		wdt_toolbar(rt.x1, rt.y1, rt.width(), e);
	}
}