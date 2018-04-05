#include "bsreq.h"
#include "collections.h"
#include "command.h"
#include "crt.h"
#include "database.h"
#include "draw.h"
#include "draw_textedit.h"
#include "draw_tree.h"
#include "settings.h"
#include "widget.h"

using namespace	draw;
using namespace database;

static struct tab_element {
	const char*		name;
	operator bool() const { return name != 0; }
} tab_element_data[] = {
	{"Реквизиты"},
	{"Права"},
};

static bool validate_header(bsval* variables) {
	return true;
}

static bool change_object(database::object& source) {
	struct context {
		database::object	object;
		controls::table		table;
		context(collection& ts1) : table(ts1) {
		}
	};
	static bsreq context_type[] = {
		BSREQ(context, object, object_type),
		BSREQ(context, table, control_type),
		{}
	};
	static widget widgets[] = {
		{WidgetField, "Наименование", "name"},
		{WidgetField, "Родитель", "parent"},
		{}
	};
	arefc<database::object*> requisites;
	context	ec(requisites); ec.object = source;
	static bsval variables[] = {{context_type, &ec}, {object_type, &ec.object}, {}};
	return draw::open(source.name, 400, 300, widgets, variables, validate_header);
}

static struct widget_database_header : controls::tree {

	void initialize() {
		addcol(WidgetImage, "image", "Изображение");
		addcol(WidgetField | ColumnSizeAuto | ColumnSmallHilite, "name", "Наименование");
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
				tree::addrow((unsigned)&e, hr ? TIGroup : 0, hr ? 1 : 0, 0);
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

	bool changing(void* object, const char* id, unsigned flags) override {
		return change_object(*((struct object*)object));
	}

	widget_database_header() {
		show_header = false;
		show_toolbar = false;
		show_border = true;
		no_change_count = true;
		element_param_have_row = true;
	}

} database_header_control;
static control::plugin plugin(database_header_control);

void initialize_database_view() {
	database_header_control.initialize();
}