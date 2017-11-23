#include "amem.h"
#include "crt.h"
#include "draw.h"
#include "control_textedit.h"
#include "control_table.h"
#include "xsfield.h"
#include "wrapper.h"

using namespace draw;

static unsigned edit_flags;
static const widget* edit_childs;
static int edit_command;

void callback_edit()
{
	field_type_s type;
	auto id = hot::name;
	auto source = hot::source;
	auto childs = edit_childs;
	char temp[8192]; temp[0] = 0;
	if(!getdata(temp, source, id, edit_childs, true, type))
		return;
	auto old_fore = draw::fore;
	draw::fore = colors::text;
	draw::controls::textedit te(temp, sizeof(temp));
	te.align = edit_flags;
	te.p1 = 0;
	te.p2 = zlen(temp);
	if(edit_command)
		hot::key = edit_command;
	if(te.editing(hot::element))
	{
		switch(type)
		{
		case FieldText:
			setdata(source, id, (int)szdup(temp), true);
			break;
		case FieldNumber:
			if(childs)
			{

			}
			else
				setdata(source, id, sz2num(temp), true);
			break;
		default:
			break;
		}
	}
	draw::fore = old_fore;
}

static bool editstart(const rect& rc, wrapper* source, const char* id, unsigned flags, const widget* childs)
{
	auto result = false;
	edit_command = 0;
	switch(hot::key&CommandMask)
	{
	case MouseMove:
	case InputIdle:
	case InputTimer:
		// Команды не влияющие на вход в режим редактирования
		break;
	case MouseLeft:
	case MouseLeftDBL:
	case MouseRight:
		edit_command = hot::key;
		result = draw::areb(rc);
		break;
	case InputSymbol:
		result = true;
		break;
	default:
		result = (hot::key&CommandMask) >= KeyLeft;
		break;
	}
	if(result)
	{
		execute(callback_edit);
		hot::name = id;
		hot::element = rc;
		hot::source = source;
		edit_flags = flags;
		edit_childs = childs;
	}
	return result;
}

static void callback_dropdown_list()
{
	static xsfield widget_type[] = {
		BSREQ(widget, label, text_type),
		{0}
	};
	auto childs = edit_childs;
	if(!childs)
		return;
	amem source((void*)childs, sizeof(childs[0]), zlen(childs));
	draw::controls::table source_table(source);
	source_table.fields = widget_type;
	source_table.pixels_per_line = texth() + 8;
	source_table.addcol(tbl_text, "label", "Заголовок", ColumnSizeAuto);
	source_table.show_header = false;
	source_table.no_change_content = true;
	source_table.no_change_max_count = true;
	source_table.no_change_order = true;
	source_table.no_change_count = true;
	source_table.hilite_rows = true;
	source_table.focused = true;
	if(!source_table.open({hot::element.x1, hot::element.y2 + 1,
		hot::element.x2,
		hot::element.y2 + (int)source.getcount()*source_table.pixels_per_line + 2}))
		return;
	auto& result = childs[source_table.current];
	//data.set(result.value);
}

static void callback_choose_list()
{
}

int wdt_field(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, const char* link, wrapper* source, int title, const widget* childs, const char* tips)
{
	field_type_s field_type;
	draw::state push;
	char number_text[32];
	auto p = getdata(number_text, source, getdatasource(id, link), childs, false, field_type);
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
			draw::execute(callback_dropdown_list);
			hot::name = id;
			edit_childs = childs;
		}
	}
	if(field_type == FieldNumber)
	{
		auto result = addbutton(rc, "+", KeyUp, "Увеличить", "-", KeyDown, "Уменьшить");
		if(result)
		{
			auto inc = 0;
			switch(result)
			{
			case 1: inc = 1; break;
			case 2: inc = -1; break;
			}
			auto value = getdata(source, getdatasource(id, link));
			setdata(source, id, value + inc);
		}
	}
	else if(field_type == FieldReference)
	{
		if(addbutton(rc, "...", F4, "Выбрать"))
		{
			draw::execute(callback_choose_list);
			hot::name = id;
		}
	}
	focusing(id, rc, flags);
	auto a = area(rc);
	bool enter_edit = false;
	if(isfocused(flags) && id)
		enter_edit = editstart(rc, source, id, flags, childs);
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