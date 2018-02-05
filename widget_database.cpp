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
		//addcol(WidgetImage, "image", "Изображение");
		addcol(WidgetField, "name", "Наименование");
		fields = database::object_type;
	}

	const char* getid() const override {
		return "headers";
	}

	char* getname(char* result) const override {
		zcpy(result, "База данных");
		return result;
	}

	const char*	gettext(char* result, void* data, const char* id) const override {
		auto pe = (database::object*)((element*)data)->param;
		if(strcmp(id, "name") == 0)
			return pe->name;
		return "";
	}

	void addrow(object& core) {
		element e = {0};
		e.param = (unsigned)&core;
		tree::addrow(e);
	}

	void addrows(object& row) {
		for(auto& e : database::objects) {
			if(e.parent == &row)
				addrow(e);
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
	}

} database_header_control;
static control::plugin plugin(database_header_control);

void initialize_database_view() {
	database_header_control.initialize();
}