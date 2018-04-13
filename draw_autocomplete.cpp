#include "crt.h"
#include "draw_list.h"
#include "screenshoot.h"

using namespace draw::controls;

void autocomplete::setrecordlist(const char* string) {
	auto index = find(string);
	if(index != -1) {
		current = index;
		ensurevisible();
	}
}

void autocomplete::updaterecords(const char* filter) {
	updaterowheight();
	if(filter)
		this->filter = filter;
	update();
	if(!filter)
		setrecordlist(filter);
}

int autocomplete::getrecordsheight() const {
	auto line_count = maximum;
	auto line_height = pixels_per_line;
	return line_height * imin(10, line_count);
}

int autocomplete::openlist(int x, int y, int width, char* buffer, const char* buffer_max) {
	draw::screenshoot push;
	rect rc;
	focused = true;
	updaterecords(false);
	while(true) {
		push.restore();
		enablefocus();
		//enablemouse(rc);
		rc.set(x, y + 2, x + width, y + 2 + getrecordsheight());
		nonclient(rc);
		if(isshowrecords()) {
			if(records->maximum) {
				records->view(rcv);
			} else
				rcv.clear();
		}
		int id = input();
		switch(id) {
		case 0:
			return false;
		case KeyEscape:
			if(records && isshowrecords()) {
				show_records = false;
				break;
			}
			return false;
		case KeyTab:
		case KeyTab | Shift:
			draw::execute(id);
			hot::key = id;
			return true;
		case KeyEnter:
			if(records && isshowrecords()) {
				auto value = records->getname(records->current);
				if(value)
					zcpy(string, value, maxlenght);
				select(0, false);
				select(zlen(string), true);
				show_records = false;
				break;
			}
			return true;
		case InputUpdate:
			// ¬ыходим, потому что ушел фокус (мен€ли размер)
			return false;
		case MouseLeft:
		case MouseLeft + Ctrl:
		case MouseLeft + Shift:
		case MouseLeftDBL:
		case MouseLeftDBL + Ctrl:
		case MouseLeftDBL + Shift:
			if(records && isshowrecords() && areb(rcv)) {
				dodialog(id);
				draw::execute(KeyEnter);
				break;
			}
			if(!areb(rc) && hot::pressed) {
				draw::execute(id);
				hot::key = id;
				return true;
			}
			break;
		default:
			dodialog(id);
			if(id == InputSymbol) {
				auto key = hot::param & 0xFFFF;
				if(key == 8 || key >= 0x20) {
					show_records = true;
					updaterecords(true);
				}
			}
			break;
		}
	}
	return -1;
}