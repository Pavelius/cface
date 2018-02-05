#include "command.h"
#include "crt.h"
#include "collections.h"
#include "database.h"
#include "draw_tree.h"
#include "settings.h"
#include "bsreq.h"

using namespace	draw;
using namespace	draw::controls;
using namespace database;

static struct widget_database_header : tree {

	void initialize() {
		addcol(WidgetImage, "image", "Изображение");
		addcol(WidgetField|ColumnSizeAuto|ColumnSmallHilite, "name", "Наименование");
		fields = database::object_type;
	}

	const char* getid() const override {
		return "headers";
	}

	char* getname(char* result) const override {
		zcpy(result, "База данных");
		return result;
	}

	bool hasrows(object& row) {
		for(auto& e : database::objects) {
			if(e.parent == &row)
				return true;
		}
		return false;
	}

	void addrows(object& row) {
		for(auto& e : database::objects) {
			if(e.parent == &row) {
				auto hr = hasrows(e);
				tree::addrow((unsigned)&e, hr ? TIGroup :0, hr ? 1 : 0, 0);
			}
		}
	}

	void expanding() override {
		if(level == 0)
			addrows(database::root);
		else {
			auto pe = (element*)rows.get(index);
			addrows(*((database::object*)pe->param));
		}
	}

	widget_database_header() {
		show_header = false;
		show_toolbar = false;
		no_change_count = true;
		element_param_have_row = true;
	}

} database_header_control;
static control::plugin plugin(database_header_control);

void initialize_database_view() {
	database_header_control.initialize();
}