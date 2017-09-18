#include "amem.h"
#include "crt.h"
#include "draw.h"
#include "widget.h"
#include "xsref.h"
#include "widget_textedit.h"
#include "widget_table.h"

using namespace draw;

bool element::editing()
{
	char temp[8192]; temp[0] = 0;
	auto old_fore = draw::fore;
	draw::fore = colors::text;
	getstring(temp, true);
	draw::controls::textedit te(temp, sizeof(temp));
	te.align = this->flags;
	te.p1 = 0; te.p2 = zlen(temp);
	auto result = te.editing(hot::param.element);
	if(result)
	{
		if(data.fields->type == text_type)
			data.set((int)szdup(temp));
		else if(data.fields->type == number_type)
			data.set(sz2num(temp));
	}
	draw::fore = old_fore;
	return result;
}

bool element::choose()
{
	if(!childs)
		return false;
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
	if(!source_table.open({rectangle.x1, rectangle.y2 + 1,
		rectangle.x2,
		rectangle.y2 + (int)source.getcount()*source_table.pixels_per_line + 2}))
		return false;
	auto& result = childs[source_table.current];
	data.set(result.value);
	return true;
}

int wdt_field(int x, int y, int width, draw::element& e)
{
	char number_text[32];
	if(!e.data.fields)
		return 0;
	draw::state push;
	e.setposition(x, y, width);
	wdt_title(x, y, e.gettitle(), e);
	x += e.title;
	width -= e.title;
	rect rc = {x, y, x + width, y + draw::texth()*e.getheight() + 8};
	e.rectangle = rc;
	if(e.isfocus())
		e.flags |= Focused;
	e.decortext();
	if(!e.isdisabled())
		draw::rectf(rc, colors::window);
	draw::rectb(rc, colors::border);
	if(e.data.fields->type == number_type)
		e.addbutton(rc, InputEditPlus, "+", KeyUp, "Увеличить", InputEditMinus, "-", KeyDown, "Уменьшить");
	if(e.childs)
		e.addbutton(rc, InputChoose, ":dropdown", F4, "Показать список");
	e.addbutton(rc, InputChoose, "...", F4, "Выбрать");
	e.focusing(rc);
	auto a = draw::area(rc);
	auto p = e.getstring(number_text, false);
	bool enter_edit = false;
	if(e.isfocused() && e.id)
		enter_edit = e.editstart(rc, InputEdit);
	if(!enter_edit)
	{
		if(e.isfocused())
			draw::texte(rc + metrics::edit, p, e.flags, 0, zlen(p));
		else
			draw::texte(rc + metrics::edit, p, e.flags, -1, -1);
		if(e.tips && a == AreaHilited)
			tooltips(e.tips);
	}
	return rc.height() + metrics::padding * 2;
}
