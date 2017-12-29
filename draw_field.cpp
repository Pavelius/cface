#include "amem.h"
#include "crt.h"
#include "draw.h"
#include "draw_textedit.h"
#include "draw_table.h"
#include "bsreq.h"

using namespace draw;

static int		edit_command;
static void(*callback_field_next)();

static void callback_field() {
	if(edit_command)
		hot::key = edit_command;
	callback_field_next();
}

static bool editstart(const rect& rc, int id, unsigned flags, void(*callback_edit)()) {
	if(!id || !callback_edit)
		return false;
	auto result = false;
	edit_command = 0;
	switch(hot::key&CommandMask) {
	case MouseMove:
	case InputIdle:
	case InputTimer:
	case KeyTab:
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
	if(result) {
		execute(callback_field);
		callback_field_next = callback_edit;
		hot::param = id;
		hot::element = rc;
	}
	return result;
}

//static void callback_dropdown_list()
//{
//	static xsfield widget_type[] = {
//		BSREQ(widget, label, text_type),
//		{0}
//	};
//	auto childs = edit_childs;
//	if(!childs)
//		return;
//	amem source((void*)childs, sizeof(childs[0]), zlen(childs));
//	draw::controls::table source_table(source);
//	source_table.fields = widget_type;
//	source_table.pixels_per_line = texth() + 8;
//	source_table.addcol(WidgetField | ColumnSizeAuto, "label", "Заголовок");
//	source_table.show_header = false;
//	source_table.no_change_content = true;
//	source_table.no_change_order = true;
//	source_table.no_change_count = true;
//	source_table.hilite_rows = true;
//	source_table.focused = true;
//	if(!source_table.open({hot::element.x1, hot::element.y2 + 1,
//		hot::element.x2,
//		hot::element.y2 + (int)source.getcount()*source_table.pixels_per_line + 2}))
//		return;
//	auto& result = childs[source_table.current];
//	//data.set(result.value);
//}

static void header(int& x, int y, int& width, unsigned flags, const char* label, int title) {
	if(!title)
		title = 100;
	char temp[1024];
	zcpy(temp, label, sizeof(temp) - 2);
	zcat(temp, ":");
	text(x, y + 4, temp);
	x += title;
	width -= title;
}

int draw::field(int x, int y, int width, int id, unsigned flags, const char* label, const char* tips, const char* header_label, int header_width,
	void(*callback_edit)(),
	void(*callback_list)(),
	void(*callback_choose)(),
	void(*callback_up)(),
	void(*callback_down)(),
	void(*callback_open)(),
	void(*callback_setparam)(void*), void* param) {
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	if(header_label && header_label[0])
		header(x, y, width, flags, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(!isdisabled(flags))
		draw::rectf(rc, colors::window);
	focusing(id, flags, rc);
	bool focused = isfocused(flags);
	draw::rectb(rc, colors::border);
	if(callback_list) {
		if(addbutton(rc, focused, ":dropdown", F4, "Показать список"))
			doevent(id, callback_list, callback_setparam, param);
	}
	if(callback_choose) {
		if(addbutton(rc, focused, "...", F4, "Выбрать значение"))
			doevent(id, callback_choose, callback_setparam, param);
	}
	if(callback_down || callback_up) {
		auto result = addbutton(rc, focused, "+", KeyUp, "Увеличить", "-", KeyDown, "Уменьшить");
		switch(result) {
		case 1:
			if(callback_down)
				doevent(id, callback_down, callback_setparam, param);
			break;
		case 2:
			if(callback_up)
				doevent(id, callback_up, callback_setparam, param);
			break;
		}
	}
	if(callback_open) {
		if(addbutton(rc, focused, "...", F4, "Выбрать"))
			doevent(id, callback_open, callback_setparam, param);
	}
	auto a = area(rc);
	bool enter_edit = false;
	if(focused)
		enter_edit = editstart(rc, id, flags, callback_edit);
	if(!enter_edit) {
		if(label) {
			if(isfocused(flags))
				draw::texte(rc + metrics::edit, label, flags, 0, zlen(label));
			else
				draw::texte(rc + metrics::edit, label, flags, -1, -1);
		}
		if(tips && a == AreaHilited)
			tooltips(tips);
	}
	return rc.height() + metrics::padding * 2;
}