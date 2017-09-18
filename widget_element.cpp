#include "crt.h"
#include "draw.h"
#include "widget.h"

using namespace draw;

element	current_widget_element;

void draw::execute(int cid, int param, element& e)
{
	execute(cid, param);
	current_widget_element = e;
	current_widget_element.parent = 0;
}

element::element(const widget& e, element* parent) : widget(e), parent(parent), context(0), row(0), column(0), separator(false)
{
	if(parent)
		context = parent->context;
	auto pid = e.getdata();
	if(pid && context)
		data = context->getr().getvalue(pid);
	else
	{
		data.fields = 0;
		data.object = 0;
	}
}

element::element(control& e) : element()
{
	id = e.id;
	context = &e;
}

void element::clear()
{
	memset(this, 0, sizeof(element));
}

bool element::isfocus() const
{
	auto fid = getfocus();
	if(!fid && id)
	{
		setfocus(id);
		fid = getfocus();
	}
	return fid && id && strcmp(fid, id) == 0;
}

void element::focusing(const rect& rc)
{
	if(isfocus())
		flags |= Focused;
	if(id && !isdisabled())
	{
		addelement(id, rc);
		if(!isfocused() && hot::key == MouseLeft && hot::pressed && areb(rc))
			draw::execute(InputSetFocus, 0, *this);
	}
}

element* element::getparent(widget::proc type)
{
	for(auto p = parent; p; p = p->parent)
	{
		if(p->type == type)
			return p;
	}
	return 0;
}

int	element::gettitle()
{
	for(auto p = this; p; p = p->parent)
	{
		if(p->title)
			return p->title;
	}
	return 100;
}

void element::setposition(int& x, int& y, int& width)
{
	x += metrics::padding;
	y += metrics::padding;
	width -= metrics::padding * 2;
}

void element::addbutton(rect& rc, int c1, const char* t1, int k1, const char* tt1, int c2, const char* t2, int k2, const char* tt2)
{
	const int width = 20;
	rc.x2 -= width;
	auto height = rc.height() / 2;
	if(draw::buttonh({rc.x2, rc.y1, rc.x2+width, rc.y1+height},
		false, isfocused(), false, false,
		t1, k1, true, tt1))
		draw::execute(c1, 0, *this);
	if(draw::buttonh({rc.x2, rc.y1 + height, rc.x2+width, rc.y1+height*2},
		false, isfocused(), false, false,
		t2, k2, true, tt2))
		draw::execute(c2, 0, *this);
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
}

void element::addbutton(rect& rc, int c1, const char* t1, int k1, const char* tt1)
{
	const int width = 18;
	rc.x2 -= width;
	if(draw::buttonh({rc.x2, rc.y1, rc.x2+width, rc.y2},
		false, false, false, false,
		t1, k1, true, tt1))
		draw::execute(c1, 0, *this);
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
}

bool element::editstart(const rect& rc, int ev)
{
	auto msk = hot::key & 0xFFFF;
	auto result = false;
	switch(hot::key&CommandMask)
	{
	case 0: // Означает что есть другая комманда оформленная в виде execute()
	case MouseMove:
	case InputIdle:
	case InputEdit:
		// Команды не влияющие на вход в режим редактирования
		break;
	case MouseLeft:
	case MouseLeftDBL:
	case MouseRight:
		result = draw::areb(rc);
		break;
	default:
		result = (msk == InputSymbol || msk >= KeyLeft);
		break;
	}
	if(result)
	{
		draw::execute(ev, 0, *this);
		hot::element = rc;
	}
	return result;
}

void element::addvalue(int value)
{
	data.set(data.get() + value);
}

char* element::getstring(char* temp, bool to_buffer)
{
	auto value = data.get();
	if(data.fields->type == text_type)
	{
		if(to_buffer)
			zcpy(temp, (char*)value);
		return (char*)value;
	}
	temp[0] = 0;
	if(data.fields->type == number_type)
	{
		if(childs)
		{
			for(auto p = childs; *p; p++)
			{
				if(p->value == value && p->label)
				{
					zcpy(temp, p->label);
					return temp;
				}
			}
		}
		szprint(temp, "%1i", value);
	}
	return temp;
}

int	draw::view(widget::proc p, int x, int y, int width, const char* id, const char* label, unsigned flags)
{
	draw::element e;
	e.type = p;
	e.id = id;
	e.label = label;
	e.flags = flags;
	return e.type(x, y, width, e);
}

int wdt_vertical(int x, int y, int width, element& e, int* source, int count, int start)
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
		e.value = index + start;
		if(source)
			e.value = source[index];
		y += e.type(x + metrics::padding, y, width_per_column - metrics::padding, e);
		index++;
	}
	if(y2 < y)
		y2 = y;
	return y2 - y1;
}

int wdt_vertical(int x, int y, int width, element& e)
{
	if(!e.childs)
		return 0;
	int y0 = y;
	for(auto p = e.childs; *p; p++)
	{
		draw::element e1(*p, &e);
		y += p->type(x, y, width, e1);
	}
	return y - y0;
}

int wdt_horizontal(int x, int y, int width, element& e)
{
	assert(e.childs);
	assert(e.childs[0].width);
	int y0 = y;
	int mh = 0;
	int n = 0;
	for(auto p = e.childs; *p; p++)
	{
		element e1(*p, &e);
		auto w = width*p->width / 12;
		auto x1 = x + width*n / 12;
		auto h = p->type(x1, y, w, e1);
		if(h > mh)
			mh = h;
		n += p->width;
	}
	return y - y0 + mh;
}

int wdt_title(int x, int y, int width, element& e)
{
	char temp[1024];
	draw::state push;
	szprint(temp, e.label);
	zcat(temp, ":");
	draw::fore = colors::text;
	if(width == -1)
		width = draw::textw(temp) + 2;
	e.title = width;
	draw::text(x, y + 4, temp);
	return draw::texth();
}