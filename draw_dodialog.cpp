#include "draw.h"

static void(*current_execute)(); // Current executed command
static int current_focus; // Current focus element

static void callback_setfocus() {
	draw::setfocus(hot::param, true);
}

int draw::getfocus() {
	return current_focus;
}

void draw::setfocus(int value, bool instant) {
	if(instant)
		current_focus = value;
	else {
		execute(callback_setfocus);
		hot::param = value;
	}
}

bool draw::dodialog(int id) {
	auto temp_execute = current_execute;
	current_execute = 0;
	if(temp_execute) {
		temp_execute();
		return true;
	}
	switch(id) {
	case KeyTab:
	case KeyTab | Shift:
	case KeyTab | Ctrl:
	case KeyTab | Ctrl | Shift:
		id = getnext(getfocus(), id);
		if(id)
			setfocus(id, true);
		return true;
	}
	return false;
}

void draw::execute(void(*proc)()) {
	draw::execute(InputExecute);
	current_execute = proc;
}

void draw::doevent(int id, void(*callback)(), void(*callback_setparam)(void*), void* param) {
	if(callback) {
		draw::execute(callback);
		hot::param = id;
		if(callback_setparam)
			callback_setparam(param);
	} else
		draw::execute(id);
}