#include "crt.h"
#include "draw.h"
#include "widget.h"

using namespace draw;

//bool element::isfocus() const
//{
//	auto fid = getfocus();
//	if(!fid && id)
//	{
//		setfocus(id);
//		fid = getfocus();
//	}
//	return fid && id && strcmp(fid, id) == 0;
//}
//
//void element::focusing(const rect& rc)
//{
//	if(isfocus())
//		flags |= Focused;
//	if(id && !isdisabled())
//	{
//		addelement(id, rc);
//		if(!isfocused() && hot::key == MouseLeft && hot::pressed && areb(rc))
//			draw::execute(InputSetFocus, 0, *this);
//	}
//}
//
//element* element::getparent(widget::proc type)
//{
//	for(auto p = parent; p; p = p->parent)
//	{
//		if(p->type == type)
//			return p;
//	}
//	return 0;
//}
//
//int	element::gettitle()
//{
//	for(auto p = this; p; p = p->parent)
//	{
//		if(p->title)
//			return p->title;
//	}
//	return 100;
//}
//
//void element::addbutton(rect& rc, int c1, const char* t1, int k1, const char* tt1, int c2, const char* t2, int k2, const char* tt2)
//{
//	const int width = 20;
//	rc.x2 -= width;
//	auto height = rc.height() / 2;
//	if(draw::buttonh({rc.x2, rc.y1, rc.x2+width, rc.y1+height},
//		false, isfocused(), false, false,
//		t1, k1, true, tt1))
//		draw::execute(c1, 0, *this);
//	if(draw::buttonh({rc.x2, rc.y1 + height, rc.x2+width, rc.y1+height*2},
//		false, isfocused(), false, false,
//		t2, k2, true, tt2))
//		draw::execute(c2, 0, *this);
//	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
//}
//
//void element::addbutton(rect& rc, int c1, const char* t1, int k1, const char* tt1)
//{
//	const int width = 18;
//	rc.x2 -= width;
//	if(draw::buttonh({rc.x2, rc.y1, rc.x2+width, rc.y2},
//		false, false, false, false,
//		t1, k1, true, tt1))
//		draw::execute(c1, 0, *this);
//	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
//}
//
//bool element::editstart(const rect& rc, int ev)
//{
//	auto msk = hot::key & 0xFFFF;
//	auto result = false;
//	switch(hot::key&CommandMask)
//	{
//	case 0: // Означает что есть другая комманда оформленная в виде execute()
//	case MouseMove:
//	case InputIdle:
//	case InputEdit:
//		// Команды не влияющие на вход в режим редактирования
//		break;
//	case MouseLeft:
//	case MouseLeftDBL:
//	case MouseRight:
//		result = draw::areb(rc);
//		break;
//	default:
//		result = (msk == InputSymbol || msk >= KeyLeft);
//		break;
//	}
//	if(result)
//	{
//		draw::execute(ev, 0, *this);
//		hot::element = rc;
//	}
//	return result;
//}
//
//void element::addvalue(int value)
//{
//	data.set(data.get() + value);
//}
//
//char* element::getstring(char* temp, bool to_buffer)
//{
//	auto value = data.get();
//	if(data.fields->type == text_type)
//	{
//		if(to_buffer)
//			zcpy(temp, (char*)value);
//		return (char*)value;
//	}
//	temp[0] = 0;
//	if(data.fields->type == number_type)
//	{
//		if(childs)
//		{
//			for(auto p = childs; *p; p++)
//			{
//				if(p->value == value && p->label)
//				{
//					zcpy(temp, p->label);
//					return temp;
//				}
//			}
//		}
//		szprint(temp, "%1i", value);
//	}
//	return temp;
//}

int wdt_vertical(widget::proc pw, int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips,
	int* elements, int count, int start)
{
	int columns_count = imax(width / 220, 1);
	if(count < 10)
		columns_count = 1;
	int lines_per_column = count / columns_count + ((count%columns_count) ? 1 : 0);
	int width_per_column = width / columns_count;
	int x1 = x;
	int y1 = y;
	int y2 = y;
	int index = 0;
	int index2 = lines_per_column;
	while(index < count)
	{
		if(index >= index2)
		{
			x1 += width_per_column;
			if(y2 < y)
				y2 = y;
			y = y1;
			x = x1;
			index2 += lines_per_column;
		}
		auto v = index + start;
		if(elements)
			v = elements[index];
		y += pw(x + metrics::padding, y, width_per_column - metrics::padding, id, flags, label, v, source, title, childs, tips);
		index++;
	}
	if(y2 < y)
		y2 = y;
	return y2 - y1;
}

int wdt_vertical(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	if(!childs)
		return 0;
	int y0 = y;
	for(auto p = childs; *p; p++)
		y += p->type(x, y, width, p->id, p->flags, p->label, p->value, 0, p->title ? p->title : title, p->childs, p->tips);
	return y - y0;
}

int wdt_horizontal(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	if(!childs || !childs[0].width)
		return 0;
	int y0 = y;
	int mh = 0;
	int n = 0;
	for(auto p = childs; *p; p++)
	{
		auto w = width*p->width / 12;
		auto x1 = x + width*n / 12;
		auto h = p->type(x1, y, w, p->id, p->flags, p->label, p->value, 0, p->title ? p->title : title, p->childs, p->tips);
		if(h > mh)
			mh = h;
		n += p->width;
	}
	return y - y0 + mh;
}

int wdt_title(int& x, int y, int& width, unsigned flags, const char* label, int title)
{
	char temp[1024];
	draw::state push;
	szprint(temp, label);
	zcat(temp, ":");
	decortext(flags);
	text(x, y + 4, temp);
	x += title;
	width -= title;
	return draw::texth();
}

int wdt_group(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	int y0 = y;
	setposition(x, y, width); // Первая рамка (может надо двойную ?)
	int x1 = x, y1 = y, w1 = width;
	setposition(x, y, width); // Отступ от рамки
	draw::state push;
	draw::font = metrics::font;
	if(label)
		y += draw::texth() + metrics::padding * 2;
	if(childs[0].width)
		y += wdt_horizontal(x, y, width, id, flags, label, value, source, title, childs, tips);
	else
		y += wdt_vertical(x, y, width, id, flags, label, value, source, title, childs, tips);
	if(label)
	{
		color c1 = colors::border.mix(colors::window, 128);
		color c2 = c1.darken();
		draw::gradv({x1, y1, x1 + w1, y1 + draw::texth() + metrics::padding * 2}, c1, c2);
		draw::fore = colors::text.mix(c1, 96);
		draw::text(x1 + (w1 - draw::textw(label)) / 2, y1 + metrics::padding, label);
	}
	y += metrics::padding;
	draw::rectb({x1, y1, x1 + w1, y}, colors::border);
	return y - y0;
}

int wdt_label(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	return draw::textf(x, y, width, label) + metrics::padding * 2;
}