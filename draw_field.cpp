#include "amem.h"
#include "crt.h"
#include "draw.h"

using namespace draw;

//bool element::editing()
//{
//	char temp[8192]; temp[0] = 0;
//	auto old_fore = draw::fore;
//	draw::fore = colors::text;
//	getstring(temp, true);
//	draw::controls::textedit te(temp, sizeof(temp));
//	te.align = this->flags;
//	te.p1 = 0; te.p2 = zlen(temp);
//	auto result = te.editing(hot::element);
//	if(result)
//	{
//		if(data.fields->type == text_type)
//			data.set((int)szdup(temp));
//		else if(data.fields->type == number_type)
//			data.set(sz2num(temp));
//	}
//	draw::fore = old_fore;
//	return result;
//}
//
//bool element::choose()
//{
//	if(!childs)
//		return false;
//	amem source((void*)childs, sizeof(childs[0]), zlen(childs));
//	draw::controls::table source_table(source);
//	source_table.fields = widget_type;
//	source_table.pixels_per_line = texth() + 8;
//	source_table.addcol(tbl_text, "label", "Заголовок", ColumnSizeAuto);
//	source_table.show_header = false;
//	source_table.no_change_content = true;
//	source_table.no_change_max_count = true;
//	source_table.no_change_order = true;
//	source_table.no_change_count = true;
//	source_table.hilite_rows = true;
//	source_table.focused = true;
//	if(!source_table.open({rectangle.x1, rectangle.y2 + 1,
//		rectangle.x2,
//		rectangle.y2 + (int)source.getcount()*source_table.pixels_per_line + 2}))
//		return false;
//	auto& result = childs[source_table.current];
//	data.set(result.value);
//	return true;
//}

static bool editstart(const rect& rc, int ev, const char* id)
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
		execute(ev, hot::param);
		hot::name = id;
		hot::element = rc;
	}
	return result;
}

static void callback_dropdown_list()
{
}

static void callback_choose_list()
{
}

int wdt_field(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const draw::widget* childs, const char* tips)
{
	draw::state push;
	char number_text[32];
	auto p = getdata(number_text, source, getdatasource(id, link), childs, false);
	if(!p)
		return 0;
	setposition(x, y, width);
	wdt_title(x, y, width, flags, label, title);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(getfocus() == id)
		flags |= Focused;
	decortext(flags);
	if(!isdisabled(flags))
		draw::rectf(rc, colors::window);
	draw::rectb(rc, colors::border);
	if(childs)
	{
		if(addbutton(rc, ":dropdown", F4, "Показать список"))
		{
			draw::execute(InputDropDown);
			hot::name = id;
			hot::callback = callback_dropdown_list;
		}
	}
	else if(addbutton(rc, "...", F4, "Выбрать"))
	{
		draw::execute(InputChoose);
		hot::name = id;
		hot::callback = callback_choose_list;
	}
	focusing(id, rc, flags);
	auto a = area(rc);
	bool enter_edit = false;
	if(isfocused(flags) && id)
		enter_edit = editstart(rc, InputEdit, id);
	if(!enter_edit)
	{
		if(isfocused(flags))
			draw::texte(rc + metrics::edit, p, flags, 0, zlen(p));
		else
			draw::texte(rc + metrics::edit, p, flags, -1, -1);
		if(tips && a == AreaHilited)
			tooltips(tips);
	}
	return rc.height() + metrics::padding * 2;
}
