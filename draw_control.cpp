#include "crt.h"
#include "draw.h"

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
		//draw::rectf(rc, colors::form);
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

void control::view(rect rc, bool show_toolbar)
{
	draw::state push;
	struct rect rt = {rc.x1, rc.y1, rc.x2, rc.y1};
	auto commands = gettoolbar();
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
	// Теперь мы имеем область элемента
	if(areb(rc))
	{
		hot::element = rc;
		if(!disabled)
		{
			if(!hot::pressed && hot::key == MouseRight)
				draw::execute(InputMenu);
		}
	}
	// Перед выводом настроим разные элементы.
	prerender();
	// Обновим цвет элемента, который может именился
	// Процедура 'background' может изменить рамку элемента.
	// Поэтому только начиная отсюда она имеет корректное значение.
	nonclient(rc);
	if(rt.height())
		wdt_toolbar(rt.x1, rt.y1, rt.width(), "toolbar", 0, 0, 0, 0, 0, 0, commands, 0);
}

bool control::keyinput(int id)
{
	switch(id)
	{
	case InputMenu:
		contextmenu();
		break;
	//case InputChoose:
	//	current_widget_element.choose();
	//	break;
	//case InputEdit:
	//	current_widget_element.editing();
	//	break;
	//case InputEditPlus:
	//	current_widget_element.addvalue(1);
	//	break;
	//case InputEditMinus:
	//	current_widget_element.addvalue(-1);
	//	break;
	case KeyTab:
	case KeyTab | Shift:
		setfocus(getnext(getfocus(), id));
		break;
	//case InputExecute:
	//	execute(current_widget_element.id, true);
	//	break;
	case InputSetFocus:
		draw::setfocus(hot::name);
		break;
	//case InputSetValue:
	//	current_widget_element.data.set(hot::param);
	//	break;
	default:
		//pc = getcommands()->findbykey(id);
		//if(pc)
		//{
		//	pc->type(this, true);
		//	break;
		//}
		return false;
	}
	return true;
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