#include "crt.h"
#include "draw.h"
#include "draw_textedit.h"
#include "draw_list.h"
#include "screenshoot.h"

using namespace draw;
using namespace draw::controls;

static bool isspace(char sym) {
	return sym == ' ' || sym == 10 || sym == 13 || sym == 9;
}

textedit::textedit(char* string, unsigned maxlenght, bool select_text) : string(string),
maxlenght(maxlenght),
p1(0), p2(-1),
readonly(false),
rctext(metrics::edit),
records(0),
update_records(true),
show_records(true),
cashed_width(-1),
cashed_string(0),
cashed_origin(0) {
	wheels.y = 8;
	if(select_text)
		select(zlen(string), true);
}

int	textedit::getbegin() const {
	if(p2 == -1)
		return p1;
	return imin(p1, p2);
}

int	textedit::getend() const {
	if(p2 == -1)
		return p1;
	return imax(p1, p2);
}

void textedit::clear() {
	if(p2 != -1 && p1 != p2) {
		char* s1 = string + getbegin();
		char* s2 = string + getend();
		while(*s2)
			*s1++ = *s2++;
		*s1 = 0;
		if(p1 > p2)
			p1 = p2;
		if(records)
			update_records = true;
	}
	p2 = -1;
}

void textedit::paste(const char* input) {
	clear();
	int i1 = zlen(string);
	int i2 = zlen(input);
	memmove(string + p1 + i2, string + p1, (i1 - p1 + 1) * sizeof(char));
	memcpy(string + p1, input, i2);
	select(p1 + i2, false);
	if(records)
		update_records = true;
}

void textedit::correct() {
	int lenght = zlen(string);
	if(p2 != -1 && p2 > lenght)
		p2 = lenght;
	if(p1 > lenght)
		p1 = lenght;
	if(p1 < 0)
		p1 = 0;
}

void textedit::ensurevisible(int linenumber) {
}

void textedit::select(int index, bool shift) {
	int lenght = zlen(string);
	if(index < 0)
		index = 0;
	else if(index > lenght)
		index = lenght;
	if(shift) {
		if(p2 == -1)
			p2 = p1;
	} else
		p2 = -1;
	p1 = index;
}

void textedit::left(bool shift, bool ctrl) {
	int n = p1;
	if(!ctrl)
		n -= 1;
	else {
		for(; n > 0 && isspace(string[n - 1]); n--);
		for(; n > 0 && !isspace(string[n - 1]); n--);
	}
	select(n, shift);
}

void textedit::right(bool shift, bool ctrl) {
	int n = p1;
	if(!ctrl)
		n += 1;
	else {
		for(; string[n] && !isspace(string[n]); n++);
		for(; string[n] && isspace(string[n]); n++);
	}
	select(n, shift);
}

int	textedit::lineb(int index) const {
	return draw::textlb(string, index, cashed_width);
}

int	textedit::linee(int index) const {
	auto line_count = 0;
	auto line_start = draw::textlb(string, index, cashed_width, 0, &line_count);
	auto n = line_start + line_count;
	if(string[n] == 0)
		return n;
	if(n)
		return n - 1;
	return 0;
}

int	textedit::linen(int index) const {
	int result = 0;
	draw::textlb(string, index, cashed_width, &result);
	return result;
}

point textedit::getpos(rect rc, int index, unsigned state) const {
	auto line_number = 0;
	auto line_count = 0;
	auto line_start = draw::textlb(string, index, rc.width(), &line_number, &line_count);
	auto p = string + line_start;
	auto x1 = aligned(rc.x1, rc.width(), state, textw(p, line_count));
	auto y1 = rc.y1 + draw::alignedh(rc, string, state);
	return{
		(short)(x1 + textw(string + line_start, index - line_start)),
		(short)(y1 + line_number*texth() - origin.y)
	};
}

int	textedit::hittest(rect rc, point pt, unsigned state) const {
	return draw::hittest({rc.x1, rc.y1 - origin.y, rc.x2, rc.y2}, string, state, pt);
}

void textedit::cashing(rect rc) {
	rcclient = rc;
	if(rc.width() != cashed_width) {
		cashed_width = rc.width();
		draw::state push;
		draw::setclip({0, 0, 0, 0});
		maximum.y = texte({0, 0, cashed_width, 256 * 256 * 256 * 64}, string, 0, -1, -1);
	}
}

bool textedit::isshowrecords() const {
	return show_records
		&& getrecordsheight() != 0;
}

void textedit::invalidate() {
	cashed_width = -1;
}

void textedit::background(rect& rc) {
	scrollable::background(rc);
	rc = rc + rctext;
}

void textedit::redraw(rect rc) {
	cashing(rc);
	if(focused) {
		auto ev = hot::key&CommandMask;
		if((ev == MouseMove || ev == MouseLeft || ev == MouseLeftDBL || ev == MouseRight) && draw::mouseinput && hot::pressed) {
			int lenght = zlen(string);
			int index = hittest(rc, hot::mouse, align);
			if(index == -3)
				index = lenght;
			else if(index == -2)
				index = 0;
			if(index >= 0) {
				switch(hot::key&CommandMask) {
				case MouseMove:
					select(index, true);
					break;
				case MouseLeftDBL:
					select(index, false);
					left(false, true);
					right(true, true);
					break;
				default:
					select(index, (hot::key&Shift) != 0);
					break;
				}
			}
		}
		rc.y1 -= origin.y;
		texte(rc, string, align, p1, p2);
	}
}

int textedit::getrecordsheight() const {
	if(!records)
		return 0;
	auto line_count = records->maximum;
	auto line_height = records->pixels_per_line;
	return line_height*imin(10, line_count);
}

void textedit::setrecordlist(const char* string) {
	auto index = records->find(string);
	if(index != -1) {
		records->current = index;
		ensurevisible(records->current);
	}
}

void textedit::updaterecords(bool setfilter) {
	if(!records)
		return;
	records->updaterowheight();
	if(setfilter)
		records->filter = string;
	records->update();
	if(!setfilter)
		setrecordlist(string);
}

bool textedit::editing(rect rco) {
	draw::screenshoot push;
	rect rcv, rc = rco + rctext;
	focused = true;
	updaterecords(false);
	while(true) {
		push.restore();
		enablefocus();
		enablemouse(rc);
		nonclient(rc);
		if(isshowrecords()) {
			if(records->maximum) {
				rcv.set(rco.x1, rco.y2 + 2, imin(rco.x1 + 300, rco.x2), rco.y2 + 2 + getrecordsheight());
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
			// �������, ������ ��� ���� ����� (������ ������)
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
	return false;
}

void textedit::keydown(int id) {
	if(isshowrecords()) {
		records->keydown(id);
		return;
	}
	auto pt = getpos(rcclient, p1, align);
	auto i = hittest(rcclient, {pt.x, (short)(pt.y + texth())}, align);
	if(i == -3)
		i = linee(linee(p1) + 1);
	if(i >= 0)
		select(i, (id & Shift) != 0);
}

void textedit::keyup(int id) {
	auto pt = getpos(rcclient, p1, align);
	auto i = hittest(rcclient, {pt.x, (short)(pt.y - texth())}, align);
	if(i == -3)
		i = linee(lineb(p1) - 1);
	if(i >= 0)
		select(i, (id & Shift) != 0);
}

void textedit::inputsymbol(int id, int symbol) {
	char temp[8];
	if(hot::param < 0x20 || readonly)
		return;
	paste(szput(temp, hot::param));
}

void textedit::keybackspace(int id) {
	if(readonly)
		return;
	if((p2 == -1 || p1 == p2) && p1 > 0)
		select(p1 - 1, true);
	clear();
}

void textedit::keydelete(int id) {
	if(readonly)
		return;
	if(p2 == -1 || p1 == p2)
		select(p1 + 1, true);
	clear();
}

unsigned textedit::select_all(bool run) {
	if(run) {
		select(0, false);
		select(zlen(string), true);
	}
	return 0;
}

void textedit::keyright(int id) {
	right((id&Shift) != 0, (id&Ctrl) != 0);
}

void textedit::keyleft(int id) {
	left((id&Shift) != 0, (id&Ctrl) != 0);
}

void textedit::keyhome(int id) {
	select(lineb(p1), (id&Shift) != 0);
}

void textedit::keyend(int id) {
	select(linee(p1), (id&Shift) != 0);
}

unsigned textedit::copy(bool run) {
	if(p2 == -1 || p1 == p2)
		return Disabled;
	if(run) {
		char* s1 = string + imin(p1, p2);
		char* s2 = string + imax(p1, p2);
		clipboard::copy(s1, s2 - s1);
	}
	return 0;
}

unsigned textedit::paste(bool run) {
	if(p1 == -1 || readonly)
		return Disabled;
	if(run) {
		clear();
		auto p = clipboard::paste();
		if(p)
			paste(p);
		delete p;
	}
	return 0;
}

//	case Ctrl + Alpha + 'X':
//		if(p2 != -1 && p1 != p2)
//		{
//			char* s1 = string + imin(p1, p2);
//			char* s2 = string + imax(p1, p2);
//			clipboard::copy(s1, s2 - s1);
//		}
//		if(!readonly)
//			clear();
//		break;
//	case Ctrl + Alpha + 'C':
//		break;
//	case Ctrl + Alpha + 'V':
//		if(p1 == -1 || readonly)
//			return true;
//		n = clipboard::paste(0, maxlenght - p1);
//		if(n > (int)sizeof(char))
//		{
//			clear();
//			int i = zlen(string);
//			int x = (n / sizeof(string[0])) - 1;
//			memmove(string + p1 + x, string + p1, (i - p1 + x) * sizeof(string[0]));
//			clipboard::paste(string + p1, x * sizeof(string[0]));
//			select(p1 + x, false);
//		}
//		break;

control::command textedit::commands[] = {
	CONTROL_ICN(copy, "����������", Ctrl + Alpha + 'C', 4),
	CONTROL_ICN(paste, "��������", Ctrl + Alpha + 'V', 5),
	{0}
};