#include "command.h"
#include "crt.h"
#include "collections.h"
#include "draw_table.h"
#include "io.h"
#include "settings.h"
#include "xsfield.h"

using namespace draw;
using namespace draw::controls;

static struct widget_io_plugin_viever : tableref {

	void initialize() {
		static xsfield control_type[] = {
			BSREQ(io::plugin, name, text_type),
			BSREQ(io::plugin, fullname, text_type),
			BSREQ(io::plugin, filter, text_type),
			{0},
		};
		show_toolbar = false;
		no_change_order = true;
		no_change_count = true;
		fields = control_type;
		//addcol(WidgetLabel, "name", "Наименование");
		addcol(WidgetLabel, "fullname", "Наименование");
		addcol(WidgetLabel, "filter", "Формат файлов");
		for(auto p = io::plugin::first; p; p = p->next)
			addelement(p);
	}
	
	widget_io_plugin_viever() {
	}

} control_viewer;

COMMAND(settings_initialize) {
	control_viewer.initialize();
	if(!control_viewer.rows.getcount())
		return;
	settings& e1 = settings::root.gr("Дополнения").add("Экспорт/Импорт", control_viewer);
}