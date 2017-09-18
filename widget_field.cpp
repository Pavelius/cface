#include "amem.h"
#include "crt.h"
#include "draw.h"
#include "widget.h"

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

int wdt_field(int x, int y, int width, const char* id, unsigned flags, const char* label, int value, void* source, int title, const widget* childs, const char* tips)
{
	draw::state push;
	//char number_text[32];
	setposition(x, y, width);
	wdt_title(x, y, title, flags, label, title);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(getfocus() == id)
		flags |= Focused;
	decortext(flags);
	if(!isdisabled(flags))
		draw::rectf(rc, colors::window);
	draw::rectb(rc, colors::border);
	//if(childs)
	//	addbutton(rc, InputChoose, ":dropdown", F4, "Показать список");
	//e.addbutton(rc, InputChoose, "...", F4, "Выбрать");
	focusing(id, rc, flags);
	auto a = area(rc);
	auto p = "";// e.getstring(number_text, false);
	bool enter_edit = false;
	//if(isfocused(flags) && e.id)
	//	enter_edit = e.editstart(rc, InputEdit);
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
