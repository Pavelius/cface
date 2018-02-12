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
		database::object	copy;
		controls::table		table;
		context(collection& ts1) : table(ts1) {
		}
	};
	static bsreq context_type[] = {
		BSREQ(context, copy, object_type),
		BSREQ(context, table, control_type),
		{}
	};
	static widget widgets[] = {
		{WidgetField, "Наименование", "name"},
		{WidgetField, "Родитель", "parent"},
		{}
	};
	arefc<database::object*> requisites;
	context	ec(requisites); ec.copy = source;
	static bsval variables[] = {{context_type, &ec}, {object_type, &ec.copy}, {}};
	return draw::open(source.name, 400, 300, widgets, variables, validate_header);
}

//static bool change_object2(database::object& source) {
//	window dc(-1, -1, 400, 300, WFMinmax | WFResize, 0, "HeaderForm");
//	setcaption(source.name);
//	const int dy = texth() + 8 + metrics::padding * 2;
//	tab_element* elements[16] = {0};
//	for(auto& e : tab_element_data)
//		zcat(elements, &e);
//	auto current_tab = 0;
//	setfocus(0, true);
//	adatc<tab_element, 16> table_elements;
//	table test_table(table_elements);
//	auto tab_height = draw::texth() + metrics::padding * 4;
//	while(true) {
//		rect rc = {0, 0, getwidth(), getheight() - dy};
//		rectf({0, 0, getwidth(), getheight()}, colors::form);
//		rc.offset(metrics::padding * 2);
//		// Вывод элементов
//		rc.y1 += draw::field(rc.x1, rc.y1, rc.width(), (int)&source.name, 0, source.name, 0, "Наименование", 100, callback_edit_text);
//		draw::tabs({rc.x1, rc.y1, rc.x1 + rc.width(), rc.y1 + tab_height},
//			false, false, (void**)elements, 0, zlen(elements), current_tab, 0, gettabname);
//		rc.y1 += tab_height + metrics::padding;
//		test_table.view(rc);
//		// Вывод подвала
//		rc.y1 = getheight() - dy - metrics::padding;
//		rc.x2 = getwidth() - metrics::padding;
//		rc.x1 = rc.x2 - 100; button(rc.x1, rc.y1, 100, KeyEscape, 0, "Отменить");
//		rc.x1 = rc.x1 - 100; button(rc.x1, rc.y1, 100, KeyEnter, 0, "OK");
//		int id = input();
//		switch(id) {
//		case 0:
//		case KeyEscape:
//			return false;
//		case KeyEnter:
//			return true;
//		default:
//			dodialog(id);
//			break;
//		}
//	}
//}

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
		no_change_count = true;
		element_param_have_row = true;
	}

} database_header_control;
static control::plugin plugin(database_header_control);

void initialize_database_view() {
	database_header_control.initialize();
}