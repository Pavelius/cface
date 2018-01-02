#include "crt.h"
#include "command.h"
#include "draw.h"
#include "draw_control.h"
#include "hotproc.h"
#include "widget.h"

using namespace		draw;

extern rect			sys_static_area;
sprite*				metrics::toolbar = (sprite*)loadb("toolbar.pma");
sprite*				metrics::tree = (sprite*)loadb("tree.pma");
control::plugin*	control::plugin::first;
static const char*	current_name;

struct control_state {
	control*		activity;
	control*		focus;
	control*		mouse;
	control_state();
	~control_state();
} cstate;

control_state::control_state() {
	memcpy(this, &cstate, sizeof(*this));
}

control_state::~control_state() {
	memcpy(&cstate, this, sizeof(*this));
}

static void callback_invoke() {
	cstate.activity->execute(current_name);
}

static void callback_context_menu() {
	draw::updatewindow();
	hot::pressed = false;
	cstate.activity->contextmenu();
}

control::plugin::plugin(control& element) : element(element) {
	seqlink(this);
}

const control::plugin* control::plugin::find(const char* id) {
	for(auto p = first; p; p = p->next) {
		auto eid = p->element.getid();
		if(!eid)
			continue;
		if(strcmp(eid, id) == 0)
			return p;
	}
	return 0;
}

char* control::getname(char* result) const {
	return result;
}

char* control::getdescription(char* result) const {
	return result;
}

color control::getcolor(color normal) const {
	if(disabled)
		return normal.mix(colors::window);
	return normal;
}

void control::background(rect& rc) {
	if(!disabled && show_background)
		draw::rectf(rc, colors::window);
	if(show_border)
		draw::rectb(rc, getcolor(colors::border));
}

void control::nonclient(rect rc) {
	redraw(rc);
}

bool control::open(const char* title) {
	return open(title, WFMinmax | WFResize, 640, 480);
}

bool control::open(const char* title, unsigned state, int width, int height) {
	control_state push;
	draw::window dc(-1, -1, width, height, state);
	if(title)
		draw::setcaption(title);
	focused = true;
	auto result = false;
	while(true) {
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form);
		rc.offset(metrics::padding * 2);
		view(rc, show_toolbar);
		int id = draw::input();
		if(id == KeyEscape || !id)
			break;
		dodialog(id);
	}
	return result;
}

bool control::open(rect rc) {
	control_state push;
	sys_static_area.set(0, 0, draw::getwidth(), draw::getheight());
	focused = true;
	while(true) {
		view(rc);
		int id = draw::input();
		switch(id) {
		case KeyEscape:
		case InputUpdate:
		case 0:
			return false;
		case MouseLeft:
			if(hot::pressed) {
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
		dodialog(id);
	}
}

void control::mouseright(point position, int id, bool pressed) {
	// Делаем вызов процедуры после перерисовки,
	// Иначе будут некрасивые артефакты
	draw::execute(callback_context_menu);
	cstate.activity = this;
	hot::key = MouseRight;
}

void control::enablefocus() {
	if(focused)
		cstate.focus = this;
}

void control::enablemouse(const rect& rc) {
	if(hot::key >= FirstMouse && hot::key <= LastMouse && hot::mouse.in(rc))
		cstate.mouse = this;
}

void control::view(rect rc, bool show_toolbar) {
	draw::state push;
	struct rect rt = {rc.x1, rc.y1, rc.x2, rc.y1};
	auto commands = getcommands();
	if(show_toolbar && metrics::toolbar && commands && this->show_toolbar) {
		rt.y2 += metrics::toolbar->get(0).sy + 4;
		rc.y1 += rt.height() + metrics::padding;
	}
	enablefocus();
	background(rc);
	prerender();
	enablemouse(rc);
	// Обновим цвет элемента, который может именился
	// Процедура 'background' может изменить рамку элемента.
	// Поэтому только начиная отсюда она имеет корректное значение.
	nonclient(rc);
	if(rt.height())
		render(rt.x1, rt.y1, rt.width(), commands);
}

//int wdt_separator(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, control* source, int title, const widget* childs, const char* tips)
//{
//	auto height = metrics::toolbar->height;
//	//if(e.separator && *e.separator)
//	//	return 0;
//	rectf({x + 2, y + 2, x + 3, y + height - 2}, colors::border);
//	//if(e.separator)
//	//	*e.separator = true;
//	return height;
//}

void control::invoke(const char* name) const {
	draw::execute(callback_invoke);
	cstate.activity = const_cast<control*>(this);
	current_name = name;
}

int control::render(int x, int y, int width, unsigned flags, const control::command& e) const {
	rect rc = {x, y, x + width, y + width};
	if(tool(rc, isdisabled(flags), false, true))
		invoke(e.id);
	switch(e.view) {
	case ViewIcon:
		image(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2,
			metrics::toolbar, e.icon, 0,
			(isdisabled(flags)) ? 0x80 : 0xFF);
		break;
	}
	if(areb(rc)) {
		auto name = e.label;
		if(name) {
			if(e.key) {
				char temp[128];
				tooltips("%1 (%2)", name, key2str(temp, e.key));
			} else
				tooltips(name);
		}
		statusbar("Выполнить команду '%1'", name);
	}
	return width;
}

int	control::render(int x, int y, int width, const control::command* commands) const {
	if(!commands)
		return 0;
	if(!metrics::toolbar)
		return 0;
	int x2 = x + width - 6;
	auto height = metrics::toolbar->get(0).getrect(0, 0, 0).height() + 4;
	for(auto p = commands; *p; p++) {
		if(!p->type)
			continue;
		if(p->view == HideCommand)
			continue;
		auto width = height;
		if(x + width > x2) {
			// wdt_dropdown(x, y, 6, "toolbar_dropdown", 0, 0, 0, 0, source, 0, p, 0);
			break;
		}
		render(x, y, width, (((control*)this)->*p->type)(false), *p);
		x += width;
	}
	return height + metrics::padding * 2;
}

const control::command* control::command::find(const char* id) const {
	auto p = this;
	if(!p)
		return 0;
	while(*p) {
		if(p->child) {
			auto v = p->child->find(id);
			if(v)
				return v;
		} else if(strcmp(p->id, id) == 0)
			return p;
		p++;
	}
	return 0;
}

const control::command* control::command::find(int id) const {
	auto p = this;
	if(!p)
		return 0;
	while(*p) {
		if(p->child) {
			auto v = p->child->find(id);
			if(v)
				return v;
		} else if(p->key == id)
			return p;
		p++;
	}
	return 0;
}

unsigned control::execute(const char* id, bool run) {
	auto p = getcommands()->find(id);
	if(p)
		return (this->*p->type)(run);
	return 0;
}

void control::contextmenu() {
	menu e;
	int elements = 0;
	auto p = getcommands();
	if(!p)
		return;
	for(; *p; p++) {
		e.add(p->id, this);
		elements++;
	}
	auto result = (control::command*)e.choose(hot::mouse.x, hot::mouse.y);
	if(result)
		(this->*result->type)(true);
}

bool control::dodialog(int id) {
	if(!id)
		return false;
	const command* pc;
	auto temp_focus = cstate.focus;
	auto temp_mouse = cstate.mouse;
	cstate.focus = 0;
	cstate.mouse = 0;
	if(draw::dodialog(id))
		return true;
	auto key = id & 0xFFFF;
	if(key >= FirstMouse && key <= LastMouse) {
		if(!temp_mouse)
			return false;
		switch(key) {
		case MouseLeft:
			temp_mouse->mouseleft(hot::mouse, id, hot::pressed);
			break;
		case MouseRight:
			temp_mouse->mouseright(hot::mouse, id, hot::pressed);
			break;
		case MouseWheelDown:
			temp_mouse->mousewheel(hot::mouse, id, 1);
			break;
		case MouseWheelUp:
			temp_mouse->mousewheel(hot::mouse, id, -1);
			break;
		case MouseMove:
			temp_mouse->mousemove(hot::mouse, id);
			break;
		case MouseLeftDBL:
			temp_mouse->mouseleftdbl(hot::mouse, id);
			break;
		default:
			return false;
		}
		return true;
	}
	if(!temp_focus)
		return false;
	switch(key) {
	case KeyLeft: temp_focus->keyleft(id); break;
	case KeyRight: temp_focus->keyright(id); break;
	case KeyUp: temp_focus->keyup(id); break;
	case KeyDown: temp_focus->keydown(id); break;
	case KeyHome: temp_focus->keyhome(id); break;
	case KeyEnd: temp_focus->keyend(id); break;
	case KeyPageDown: temp_focus->keypagedown(id); break;
	case KeyPageUp: temp_focus->keypageup(id); break;
	case KeyEnter: temp_focus->keyenter(id); break;
	case KeySpace: temp_focus->keyspace(id); break;
	case KeyEscape: temp_focus->keyescape(id); break;
	case KeyDelete: temp_focus->keydelete(id); break;
	case KeyBackspace: temp_focus->keybackspace(id); break;
	case KeyTab: temp_focus->keytab(id); break;
	case InputTimer: temp_focus->inputtimer(); break;
	case InputSymbol: temp_focus->inputsymbol(id, hot::param); break;
	case InputUpdate: temp_focus->inputupdate(); break;
	case InputIdle: temp_focus->inputidle(); break;
	default:
		pc = temp_focus->getcommands();
		if(pc) {
			pc = pc->find(id);
			if(pc) {
				(temp_focus->*pc->type)(true);
				break;
			}
		}
		if(true) {
			auto pc = hotproc::plugin::find(id);
			if(pc) {
				pc->proc();
				break;
			}
		}
		return false;
	}
	return true;
}