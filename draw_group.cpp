#include "crt.h"
#include "draw.h"

using namespace draw;

//
//void element::addvalue(int value)
//{
//	data.set(data.get() + value);
//}
//

int wdt_vertical(widget::proc pw, int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips,
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
		y += pw(x + metrics::padding, y, width_per_column - metrics::padding, id, flags, label, v, link, source, title, childs, tips);
		index++;
	}
	if(y2 < y)
		y2 = y;
	return y2 - y1;
}

int wdt_vertical(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
{
	if(!childs)
		return 0;
	int y0 = y;
	for(auto p = childs; *p; p++)
		y += p->type(x, y, width, p->id, p->flags, p->label, p->value, p->link, source, p->title ? p->title : title, p->childs, p->tips);
	return y - y0;
}

int wdt_horizontal(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
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
		auto h = p->type(x1, y, w, p->id, p->flags, p->label, p->value, p->link, source, p->title ? p->title : title, p->childs, p->tips);
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

int wdt_group(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
{
	int y0 = y;
	setposition(x, y, width); // Первая рамка (может надо двойную ?)
	int x1 = x, y1 = y, w1 = width;
	setposition(x, y, width); // Отступ от рамки
	draw::state push;
	draw::font = metrics::font;
	if(label)
		y += texth() + metrics::padding * 2;
	if(childs[0].width)
		y += wdt_horizontal(x, y, width, id, flags, label, value, link, source, title, childs, tips);
	else
		y += wdt_vertical(x, y, width, id, flags, label, value, link, source, title, childs, tips);
	if(label)
	{
		color c1 = colors::border.mix(colors::window, 128);
		color c2 = c1.darken();
		gradv({x1, y1, x1 + w1, y1 + texth() + metrics::padding * 2}, c1, c2);
		fore = colors::text.mix(c1, 96);
		text(x1 + (w1 - textw(label)) / 2, y1 + metrics::padding, label);
	}
	y += metrics::padding;
	rectb({x1, y1, x1 + w1, y}, colors::border);
	return y - y0;
}

int wdt_label(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
{
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	return draw::textf(x, y, width, label) + metrics::padding * 2;
}

WIDGET(label);
WIDGET(group);
WIDGET(horizontal);
WIDGET(vertical);