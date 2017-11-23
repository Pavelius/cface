#include "crt.h"
#include "draw.h"
#include "draw_textedit.h"
#include "draw_list.h"
#include "screenshoot.h"

using namespace draw;
using namespace draw::controls;

static bool isspace(char sym)
{
	return sym == ' ' || sym == 10 || sym == 13 || sym == 9;
}

textedit::textedit(char* string, unsigned maxlenght) : string(string),
maxlenght(maxlenght),
p1(0), p2(-1),
readonly(false),
rctext(metrics::edit),
records(0),
update_records(true),
cashed_width(-1),
cashed_string(0),
cashed_origin(0)
{
	wheels.y = 8;
}

int	textedit::getbegin() const
{
	if(p2 == -1)
		return p1;
	return imin(p1, p2);
}

int	textedit::getend() const
{
	if(p2 == -1)
		return p1;
	return imax(p1, p2);
}

void textedit::clear()
{
	if(p2 != -1 && p1 != p2)
	{
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

void textedit::paste(const char* input)
{
	clear();
	int i1 = zlen(string);
	int i2 = zlen(input);
	memmove(string + p1 + i2, string + p1, (i1 - p1 + 1) * sizeof(char));
	memcpy(string + p1, input, i2);
	select(p1 + i2, false);
	if(records)
		update_records = true;
}

void textedit::correct()
{
	int lenght = zlen(string);
	if(p2 != -1 && p2 > lenght)
		p2 = lenght;
	if(p1 > lenght)
		p1 = lenght;
	if(p1 < 0)
		p1 = 0;
}

void textedit::ensurevisible(int linenumber)
{
}

void textedit::select(int index, bool shift)
{
	int lenght = zlen(string);
	if(index < 0)
		index = 0;
	else if(index > lenght)
		index = lenght;
	if(shift)
	{
		if(p2 == -1)
			p2 = p1;
	}
	else
		p2 = -1;
	p1 = index;
}

void textedit::left(bool shift, bool ctrl)
{
	int n = p1;
	if(!ctrl)
		n -= 1;
	else
	{
		for(; n > 0 && isspace(string[n - 1]); n--);
		for(; n > 0 && !isspace(string[n - 1]); n--);
	}
	select(n, shift);
}

void textedit::right(bool shift, bool ctrl)
{
	int n = p1;
	if(!ctrl)
		n += 1;
	else
	{
		for(; string[n] && !isspace(string[n]); n++);
		for(; string[n] && isspace(string[n]); n++);
	}
	select(n, shift);
}

int	textedit::lineb(int index) const
{
	return draw::textlb(string, index, cashed_width);
}

int	textedit::linee(int index) const
{
	auto line_count = 0;
	auto line_start = draw::textlb(string, index, cashed_width, 0, &line_count);
	auto n = line_start + line_count;
	if(string[n] == 0)
		return n;
	if(n)
		return n - 1;
	return 0;
}

int	textedit::linen(int index) const
{
	int result = 0;
	draw::textlb(string, index, cashed_width, &result);
	return result;
}

point textedit::getpos(rect rc, int index, unsigned state) const
{
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

int	textedit::hittest(rect rc, point pt, unsigned state) const
{
	return draw::hittest({rc.x1, rc.y1 - origin.y, rc.x2, rc.y2}, string, state, pt);
}

void textedit::cashing(rect rc)
{
	rcclient = rc;
	if(rc.width() != cashed_width)
	{
		cashed_width = rc.width();
		draw::state push;
		draw::setclip({0, 0, 0, 0});
		maximum.y = texte({0, 0, cashed_width, 256 * 256 * 256 * 64}, string, 0, -1, -1);
	}
}

void textedit::invalidate()
{
	cashed_width = -1;
}

void textedit::background(rect& rc)
{
	scrollable::background(rc);
	rc = rc + rctext;
}

void textedit::redraw(rect rc)
{
	cashing(rc);
	if(focused)
	{
		auto ev = hot::key&CommandMask;
		if((ev == MouseMove || ev == MouseLeft || ev == MouseLeftDBL || ev == MouseRight) && draw::mouseinput && hot::pressed)
		{
			int lenght = zlen(string);
			int index = hittest(rc, hot::mouse, align);
			if(index == -3)
				index = lenght;
			else if(index == -2)
				index = 0;
			if(index >= 0)
			{
				switch(hot::key&CommandMask)
				{
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

int textedit::getrecordsheight() const
{
	if(!records)
		return 0;
	auto line_count = records->maximum;
	auto line_height = records->pixels_per_line;
	return line_height*imin(10, line_count);
}

void textedit::updaterecords()
{
	if(!records)
		return;
	records->filter = string;
	records->keyinput(InputUpdate);
}

bool textedit::editing(rect rco)
{
	draw::screenshoot push;
	rect rcv, rc = rco + rctext;
	focused = true;
	updaterecords();
	while(true)
	{
		push.restore();
		nonclient(rc);
		if(isshowrecords())
		{
			if(records->maximum)
			{
				rcv.set(rco.x1, rco.y2 + 2, imin(rco.x1 + 300, rco.x2), rco.y2 + 4 + getrecordsheight());
				records->view(rcv);
			}
			else
				rcv.clear();
		}
		int id = input();
		switch(id)
		{
		case 0:
			return false;
		case KeyEscape:
			if(records && isshowrecords())
				break;
			return false;
		case KeyTab:
		case KeyTab | Shift:
			draw::execute(id);
			hot::key = id;
			return true;
		case KeyEnter:
			if(records && isshowrecords())
			{
				//auto value = records->get("current");
				//if(value)
				//	zcpy(string, value, maxlenght);
				select(0, false);
				select(zlen(string), true);
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
			if(records && isshowrecords() && areb(rcv))
			{
				records->keyinput(id);
				break;
			}
			if(!areb(rc) && hot::pressed)
			{
				draw::execute(id);
				hot::key = id;
				return true;
			}
			break;
		case MouseWheelDown:
		case MouseWheelUp:
			if(records && isshowrecords() && areb(rcv))
				records->keyinput(id);
			else
				keyinput(id);
			break;
		case KeyPageUp:
		case KeyPageDown:
			if(records && isshowrecords())
				records->keyinput(id);
			else
				keyinput(id);
			break;
		default:
			keyinput(id);
			break;
		}
	}
	return false;
}

unsigned textedit::down(control* source, bool run)
{
	auto pc = (textedit*)source;
	if(run)
	{
		if(pc->records)
			pc->records->keyinput(KeyDown);
		else
		{
			auto pt = pc->getpos(pc->rcclient, pc->p1, pc->align);
			auto i = pc->hittest(pc->rcclient, {pt.x, (short)(pt.y + texth())}, pc->align);
			if(i == -3)
				i = pc->linee(pc->linee(pc->p1) + 1);
			if(i >= 0)
				pc->select(i, false);
		}
	}
	return Executed;
}

unsigned textedit::downs(control* source, bool run)
{
	auto pc = (textedit*)source;
	if(run)
	{
		auto pt = pc->getpos(pc->rcclient, pc->p1, pc->align);
		auto i = pc->hittest(pc->rcclient, {pt.x, (short)(pt.y + texth())}, pc->align);
		if(i == -3)
			i = pc->linee(pc->linee(pc->p1) + 1);
		if(i >= 0)
			pc->select(i, true);
	}
	return Executed;
}

unsigned textedit::up(control* source, bool run)
{
	auto pc = (textedit*)source;
	if(run)
	{
		if(pc->records && pc->isshowrecords())
			pc->records->keyinput(KeyUp);
		else
		{
			auto pt = pc->getpos(pc->rcclient, pc->p1, pc->align);
			auto i = pc->hittest(pc->rcclient, {pt.x, (short)(pt.y - texth())}, pc->align);
			if(i == -3)
				i = pc->linee(pc->lineb(pc->p1) - 1);
			if(i >= 0)
				pc->select(i, false);
		}
	}
	return Executed;
}

unsigned textedit::ups(control* source, bool run)
{
	auto pc = (textedit*)source;
	if(run)
	{
		auto pt = pc->getpos(pc->rcclient, pc->p1, pc->align);
		auto i = pc->hittest(pc->rcclient, {pt.x, (short)(pt.y - texth())}, pc->align);
		if(i == -3)
			i = pc->linee(pc->lineb(pc->p1) - 1);
		if(i >= 0)
			pc->select(i, true);
	}
	return Executed;
}

unsigned textedit::symbol(control* source, bool run)
{
	char temp[8];
	auto pc = (textedit*)source;
	if(hot::param < 0x20 || pc->readonly)
		return Disabled;
	if(run)
		pc->paste(szput(temp, hot::param));
	return Executed;
}

unsigned textedit::backspace(control* source, bool run)
{
	auto pc = (textedit*)source;
	if(pc->readonly)
		return Disabled;
	if(run)
	{
		if((pc->p2 == -1 || pc->p1 == pc->p2) && pc->p1 > 0)
			pc->select(pc->p1 - 1, true);
		pc->clear();
	}
	return Executed;
}

unsigned textedit::delsym(control* source, bool run)
{
	auto pc = (textedit*)source;
	if(pc->readonly)
		return Disabled;
	if(pc->p2 == -1 || pc->p1 == pc->p2)
		pc->select(pc->p1 + 1, true);
	pc->clear();
	return Executed;
}

unsigned textedit::home(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->select(pc->lineb(pc->p1), false);
	return Executed;
}

unsigned textedit::end(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->select(pc->linee(pc->p1), false);
	return Executed;
}

static unsigned execute_text_end(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->select(zlen(pc->string), false);
	return Executed;
}

static unsigned execute_select_home(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->select(pc->lineb(pc->p1), true);
	return Executed;
}

static unsigned execute_select_end(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->select(pc->linee(pc->p1), true);
	return Executed;
}

static unsigned execute_select_all(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->select(0, false);
	pc->select(zlen(pc->string), true);
	return Executed;
}

unsigned textedit::right(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->right(false, false);
	return Executed;
}

unsigned textedit::rights(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->right(true, false);
	return Executed;
}

unsigned textedit::rightc(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->right(false, true);
	return Executed;
}

unsigned textedit::rightcs(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->right(true, true);
	return Executed;
}

unsigned textedit::left(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->left(false, false);
	return Executed;
}

unsigned textedit::lefts(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->left(true, false);
	return Executed;
}

unsigned textedit::leftc(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->left(false, true);
	return Executed;
}

unsigned textedit::leftcs(control* source, bool run)
{
	auto pc = (textedit*)source;
	pc->left(true, true);
	return Executed;
}

control::command textedit::commands[] = {
	{"super", "", 0, control::commands},
	{"backspace", "������� ������ �����", textedit::backspace, 0, {KeyBackspace}, 0, HideCommand},
	{"delete", "������� ������", textedit::delsym, 0, {KeyDelete}, 0, HideCommand},
	{"down", "������� ����", textedit::down, 0, {KeyDown}, 0, HideCommand},
	{"down_shift", "������� ����", textedit::downs, 0, {KeyDown | Shift}, 0, HideCommand},
	{"end", "� �����", textedit::end, 0, {KeyEnd}, 0, HideCommand},
	{"home", "� ������", textedit::home, 0, {KeyHome}, 0, HideCommand},
	{"left", "������������� �����", textedit::left, 0, {KeyLeft}, 0, HideCommand},
	{"left_shift", "������������� �����", textedit::lefts, 0, {KeyLeft | Shift}, 0, HideCommand},
	{"left_ctrl", "������������� �����", textedit::leftc, 0, {KeyLeft | Ctrl}, 0, HideCommand},
	{"left_ctrl_shift", "������������� �����", textedit::leftcs, 0, {KeyLeft | Ctrl | Shift}, 0, HideCommand},
	{"right", "������������� ������", textedit::right, 0, {KeyRight}, 0, HideCommand},
	{"right_shift", "������������� ������", textedit::rights, 0, {KeyRight | Shift}, 0, HideCommand},
	{"right_ctrl", "������������� ������", textedit::rightc, 0, {KeyRight | Ctrl}, 0, HideCommand},
	{"right_ctrl_shift", "������������� ������", textedit::rightcs, 0, {KeyRight | Ctrl | Shift}, 0, HideCommand},
	{"select_all", "�������� ���", execute_select_all, 0, {Ctrl | (Alpha + 'A')}, 0, HideToolbar},
	{"select_end", "�������� �� ����� ������", execute_select_end, 0, {Shift | KeyEnd}, 0, HideCommand},
	{"select_home", "�������� �� ������ ������", execute_select_home, 0, {Shift | KeyHome}, 0, HideCommand},
	{"text_end", "� ����� ������", execute_text_end, 0, {Ctrl | KeyEnd}, 0, HideCommand},
	{"symbol", "������", textedit::symbol, 0, {InputSymbol, InputSymbol | Shift}, 0, HideCommand},
	{"up", "������������� �����", textedit::up, 0, {KeyUp}, 0, HideCommand},
	{"up_shift", "������������� �����", textedit::ups, 0, {KeyUp | Shift}, 0, HideCommand},
	{0}
};

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
//		if(p2 != -1 && p1 != p2)
//		{
//			char* s1 = string + imin(p1, p2);
//			char* s2 = string + imax(p1, p2);
//			clipboard::copy(s1, s2 - s1);
//		}
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