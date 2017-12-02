#include "amem.h"
#include "crt.h"
#include "draw.h"
#include "draw_textedit.h"
#include "draw_table.h"
#include "xsfield.h"

using namespace draw;

static unsigned edit_flags;
static const widget* edit_childs;
static int edit_command;

void callback_edit()
{
	//auto id = hot::name;
	//auto source = hot::source;
	//auto childs = edit_childs;
	//char temp[8192]; temp[0] = 0;
	//if(!getdata(temp, source, id, edit_childs, true, type))
	//	return;
	//auto old_fore = draw::fore;
	//draw::fore = colors::text;
	//draw::controls::textedit te(temp, sizeof(temp));
	//te.align = edit_flags;
	//te.p1 = 0;
	//te.p2 = zlen(temp);
	//if(edit_command)
	//	hot::key = edit_command;
	//if(te.editing(hot::element))
	//{
	//	switch(type)
	//	{
	//	case FieldText:
	//		setdata(source, id, (int)szdup(temp), true);
	//		break;
	//	case FieldNumber:
	//		if(childs)
	//		{

	//		}
	//		else
	//			setdata(source, id, sz2num(temp), true);
	//		break;
	//	default:
	//		break;
	//	}
	//}
	//draw::fore = old_fore;
}

static bool editstart(const rect& rc, control* source, const char* id, unsigned flags, const widget* childs)
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
		//execute(callback_edit);
		hot::element = rc;
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
	source_table.addcol(WidgetField | ColumnSizeAuto, "label", "Заголовок");
	source_table.show_header = false;
	source_table.no_change_content = true;
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