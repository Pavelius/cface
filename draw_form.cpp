#include "crt.h"
#include "draw.h"
#include "draw_form.h"

using namespace draw;
using namespace draw::controls;

static char* get_text(char* result, void* object)
{
	if(((widget*)object)->label)
		zcpy(result, ((widget*)object)->label, 259);
	return result;
}

static void callback_setfocus()
{
	//draw::setfocus(hot::name);
	//execute(InputUpdate);
}

static void invoke_setfocus(const char* id)
{
	//execute(callback_setfocus);
	//hot::name = id;
}

unsigned form::getflags(const widget& e) const
{
	unsigned result = e.flags;
	if(disabled)
		result |= Disabled;
	if(getfocus() == (int)&e)
		result |= Focused;
	return result;
}

int form::header(int& x, int y, int& width, unsigned flags, const char* label, int title)
{
	char temp[1024];
	draw::state push;
	zcpy(temp, label, sizeof(temp)-2);
	zcat(temp, ":");
	decortext(flags);
	text(x, y + 4, temp);
	x += title;
	width -= title;
	return draw::texth();
}

int form::group(int x, int y, int width, const widget& e)
{
	int y0 = y;
	setposition(x, y, width); // Первая рамка (может надо двойную ?)
	int x1 = x, y1 = y, w1 = width;
	setposition(x, y, width); // Отступ от рамки
	draw::state push;
	draw::font = metrics::font;
	if(e.label)
		y += texth() + metrics::padding * 2;
	if(e.childs[0].width)
		y += horizontal(x, y, width, e);
	else
		y += vertical(x, y, width, e);
	if(e.label)
	{
		color c1 = colors::border.mix(colors::window, 128);
		color c2 = c1.darken();
		gradv({x1, y1, x1 + w1, y1 + texth() + metrics::padding * 2}, c1, c2);
		fore = colors::text.mix(c1, 96);
		text(x1 + (w1 - textw(e.label)) / 2, y1 + metrics::padding, e.label);
	}
	y += metrics::padding;
	rectb({x1, y1, x1 + w1, y}, colors::border);
	return y - y0;
}

int form::vertical(int x, int y, int width, const widget& e)
{
	int y0 = y;
	//for(auto p = e.childs; *p; p++)
	//	y += p->type(x, y, width, getflags(*p), p->label, p->value, getdata(*p), p->tips);
	return y - y0;
}

int form::horizontal(int x, int y, int width, const widget& e)
{
	int mh = 0;
	int n = 0;
	for(auto p = e.childs; *p; p++)
	{
		auto w = width*p->width / 12;
		auto x1 = x + width*n / 12;
		//auto h = p->type(x, y, width, getflags(*p), p->label, p->value, getdata(*p), p->tips);
		//if(h > mh)
		//	mh = h;
		n += p->width;
	}
	return y + mh;
}

int form::element(int x, int y, int width, const widget& e)
{
	return 0;
}

int form::field(int x, int y, int width, const widget& e)
{
	auto field_type = e.gettype();
	draw::state push;
	char number_text[32];
	auto p = getdata(number_text, e);
	if(!p)
		return 0;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	auto flags = getflags(e);
	decortext(flags);
	if(!isdisabled(flags))
		draw::rectf(rc, colors::window);
	draw::rectb(rc, colors::border);
	//if(e.childs)
	//{
	//	if(addbutton(rc, ":dropdown", F4, "Показать список"))
	//	{
	//		draw::execute(callback_dropdown_list);
	//		hot::element = rc;
	//		hot::name = id;
	//		edit_childs = childs;
	//	}
	//}
	//if(field_type == FieldNumber)
	//{
	//	auto result = addbutton(rc, "+", KeyUp, "Увеличить", "-", KeyDown, "Уменьшить");
	//	if(result)
	//	{
	//		auto inc = 0;
	//		switch(result)
	//		{
	//		case 1: inc = 1; break;
	//		case 2: inc = -1; break;
	//		}
	//		auto value = getdata(source, getdatasource(id, link));
	//		setdata(source, id, value + inc);
	//	}
	//}
	//else if(field_type == FieldReference)
	//{
	//	if(addbutton(rc, "...", F4, "Выбрать"))
	//	{
	//		draw::execute(callback_choose_list);
	//		hot::name = id;
	//	}
	//}
	//focusing(id, rc, flags);
	//auto a = area(rc);
	//bool enter_edit = false;
	//if(isfocused(flags) && id)
	//	enter_edit = editstart(rc, source, id, flags, childs);
	//if(!enter_edit)
	//{
	//	if(isfocused(flags))
	//		draw::texte(rc + metrics::edit, p, flags, 0, zlen(p));
	//	else
	//		draw::texte(rc + metrics::edit, p, flags, -1, -1);
	//	if(tips && a == AreaHilited)
	//		tooltips(tips);
	//}
	return rc.height() + metrics::padding * 2;
}

int form::tabs(int x, int y, int width, const widget& e)
{
	if(!e.childs)
		return 0;
	auto y0 = y;
	const int tab_height = 24 + 4;
	y += 1;
	sheetline({x, y0, x + width, y + tab_height});
	const widget* data[32];
	auto ps = data;
	auto pe = data + sizeof(data) / sizeof(data[0]);
	for(auto p = e.childs; *p; p++)
	{
		if(ps < pe)
			*ps++ = p;
	}
	auto count = ps - data;
	auto current = getdata(e);
	rect rc = {x, y, x + width, y + tab_height};
	int tabs_hilite;
	if(draw::tabs(rc, false, false, (void**)data, 0, count, current, &tabs_hilite, get_text, 0, {0,0,0,0}))
	{
		//if(tabs_hilite != -1)
		//	setdata(source, getdatasource(id, link), tabs_hilite);
	}
	y += tab_height + metrics::padding;
	auto& pw = e.childs[0];
	return 0;
	//return pw.type(x, y, width, pw.id, pw.flags, pw.label, pw.value, pw.link, source, pw.title, pw.childs, pw.tips) + (y - y0);
}

int form::decoration(int x, int y, int width, const widget& e)
{
	draw::state push;
	setposition(x, y, width);
	decortext(getflags(e));
	return draw::textf(x, y, width, e.label) + metrics::padding * 2;
}

void form::focusing(const rect& rc, unsigned& flags, const widget& e)
{
	if(flags&Disabled)
		return;
	if(!getfocus())
		setfocus((int)&e);
	if(getfocus() == (int)&e)
		flags |= Focused;
	else if(area(rc) == AreaHilitedPressed && hot::key == MouseLeft && hot::pressed)
		invoke_setfocus(id);
	addelement((int)&e, rc);
}

bool form::addbutton(rect& rc, const char* t1, int k1, const char* tt1)
{
	const int width = 18;
	rc.x2 -= width;
	auto result = draw::buttonh({rc.x2, rc.y1, rc.x2 + width, rc.y2},
		false, false, false, false,
		t1, k1, true, tt1);
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
	return result;
}

int form::addbutton(rect& rc, const char* t1, int k1, const char* tt1, const char* t2, int k2, const char* tt2)
{
	const int width = 20;
	rc.x2 -= width;
	auto height = rc.height() / 2;
	auto result = 0;
	if(draw::buttonh({rc.x2, rc.y1, rc.x2 + width, rc.y1 + height},
		false, false, false, false,
		t1, k1, true, tt1))
		result = 1;
	if(draw::buttonh({rc.x2, rc.y1 + height, rc.x2 + width, rc.y1 + height * 2},
		false, false, false, false,
		t2, k2, true, tt2))
		result = 2;
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
	return result;
}

int form::radio(int x, int y, int width, const widget& e)
{
	if(!e.label || !e.label[0])
		return 0;
	draw::state push;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, e.label);
	rc1.offset(-2);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	auto flags = getflags(e);
	decortext(flags);
	focusing(rc, flags, e);
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), width, flags, ":radio");
	bool need_select = false;
	auto a = draw::area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot::key == MouseLeft)
	{
		if(!hot::pressed)
			need_select = true;
	}
	if(isfocused(flags))
	{
		draw::rectx({rc1.x1, rc1.y1, rc1.x2, rc1.y2}, draw::fore);
		if(!isdisabled(flags) && hot::key == KeySpace)
			need_select = true;
	}
	if(need_select)
		setdata(e, e.value);
	draw::text({rc1.x1 + 2, rc1.y1 + 2, rc1.x2 - 2, rc1.y2 - 2}, e.label);
	if(e.tips && a == AreaHilited)
		tooltips(e.tips);
	return rc1.height() + metrics::padding * 2;
}

int form::check(int x, int y, int width, const widget& e)
{
	if(!e.label || !e.label[0])
		return 0;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	auto flags = getflags(e);
	if(getdata(e))
		flags |= Checked;
	draw::textw(rc1, e.label);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	focusing(rc, flags, e);
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, flags, ":check");
	decortext(flags);
	auto a = draw::area(rc);
	auto need_value = false;
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot::key == MouseLeft)
	{
		if(!hot::pressed)
			need_value = true;
		else
			invoke_setfocus(id);
	}
	if(isfocused(flags))
	{
		draw::rectx({rc1.x1 - 2, rc1.y1 - 1, rc1.x2 + 2, rc1.y2 + 1}, draw::fore);
		if(!isdisabled(flags) && hot::key == KeySpace)
			need_value;
	}
	if(need_value)
		setdata(e, ischecked(flags) ? 0 : 1);
	draw::text(rc1, e.label);
	if(e.tips && a == AreaHilited)
		tooltips(e.tips);
	return rc1.height() + metrics::padding * 2;
}

int form::button(int x, int y, int width, const widget& e)
{
	if(!e.label || !e.label[0])
		return 0;
	setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
	auto flags = getflags(e);
	focusing(rc, flags, e);
	if(buttonh({x, y, x + width, rc.y2},
		ischecked(flags), isfocused(flags), isdisabled(flags), true,
		e.label, KeyEnter, false, e.tips))
	{
		draw::execute(InputExecute);
	}
	return rc.height() + metrics::padding * 2;
}

//static void callback_setvalue(control* source, const char* id, int value)
//{
//	if(source)
//	{
//		xsref e = {source->getmeta(), source};
//		e.set(id, value);
//	}
//}
//
//static void callback_setvalue()
//{
//	callback_setvalue(hot::source, hot::name, hot::param);
//	execute(InputUpdate);
//}
//
//int	draw::getdata(control* source, const char* id)
//{
//	if(!source)
//		return 0;
//	xsref e = {source->getmeta(), source->getobject()};
//	return e.get(id);
//}
//
//void draw::setdata(control* source, const char* id, int value, bool instant)
//{
//	if(instant)
//		callback_setvalue(source, id, value);
//	else
//	{
//		execute(callback_setvalue);
//		hot::name = id;
//		hot::param = value;
//		hot::source = source;
//	}
//}
//
//char* draw::getdata(char* temp, control* source, const char* id, const widget* childs, bool to_buffer, field_type_s& type)
//{
//	temp[0] = 0;
//	type = FieldNumber;
//	auto field = source->getmeta()->find(id);
//	if(!field)
//		return 0;
//	auto value = getdata(source, id);
//	if(field->type == text_type)
//	{
//		type = FieldText;
//		if(to_buffer)
//		{
//			if(value)
//				zcpy(temp, (char*)value);
//			return temp;
//		}
//		if(!value)
//			return "";
//		return (char*)value;
//	}
//	if(field->type == number_type)
//	{
//		type = FieldNumber;
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
//	else
//	{
//		type = FieldReference;
//		xsref xr = {field->type, (void*)value};
//		auto pv = xr.get("name");
//		if(!pv)
//			return "";
//		if(!to_buffer)
//			return (char*)pv;
//		zcpy(temp, (char*)pv);
//	}
//	return temp;
//}