#include "crt.h"
#include "draw.h"
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

bool autocomplete::isshowrecords() const {
	return true;
}

int autocomplete::openlist(int x, int y, int width, char* buffer, const char* buffer_max) {
	draw::screenshoot push;
	rect rc;
	focused = true;
	updaterecords(buffer);
	while(true) {
		push.restore();
		enablefocus();
		auto show_records = false;
		if(maximum) {
			show_records = true;
			rc.set(x, y + 2, x + width, y + 2 + getrecordsheight());
			view(rc);
		}
		auto id = draw::input();
		switch(id) {
		case 0:
			return false;
		case KeyEscape:
			return -1;
		case KeyTab:
		case KeyTab | Shift:
			draw::execute(id);
			hot::key = id;
			return true;
		case KeyEnter:
			if(show_records) {
				break;
			}
			return -1;
		case InputUpdate:
			// ¬ыходим, потому что ушел фокус (мен€ли размер)
			return false;
		case MouseLeft:
		case MouseLeft + Ctrl:
		case MouseLeft + Shift:
		case MouseLeftDBL:
		case MouseLeftDBL + Ctrl:
		case MouseLeftDBL + Shift:
			if(show_records && areb(rc)) {
				dodialog(id);
				draw::execute(KeyEnter);
				break;
			}
			if(!areb(rc) && hot::pressed) {
				draw::execute(id);
				hot::key = id;
				return -1;
			}
			break;
		default:
			dodialog(id);
			if(id == InputSymbol) {
				auto key = hot::param & 0xFFFF;
				if(key == 8 || key >= 0x20) {
					show_records = true;
					updaterecords(buffer);
				}
			}
			break;
		}
	}
	return -1;
}