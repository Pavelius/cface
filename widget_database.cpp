#include "command.h"
#include "crt.h"
#include "collections.h"
#include "database.h"
#include "draw.h"
#include "draw_tree.h"
#include "settings.h"
#include "bsreq.h"

using namespace	draw;
using namespace	draw::controls;
using namespace database;

static bool change_object(database::object& source) {
	window dc(-1, -1, 400, 300, WFMinmax | WFResize, 0, "HeaderForm");
	setcaption(source.name);
	const int dy = texth() + 8 + metrics::padding * 2;
	int current_control = 0;
	setfocus(0, true);
	while(true) {
		rect rc = {0, 0, getwidth(), getheight() - dy};
		rectf({0, 0, getwidth(), getheight()}, colors::form);
		rc.offset(metrics::padding * 2);
		rc.y1 = getheight() - dy - metrics::padding;
		rc.x2 = getwidth() - metrics::padding;
		rc.x1 = rc.x2 - 100; button(rc.x1, rc.y1, 100, KeyEscape, 0, "Отменить");
		rc.x1 = rc.x1 - 100; button(rc.x1, rc.y1, 100, KeyEnter, 0, "OK");
		int id = input();
		switch(id) {
		case 0:
		case KeyEscape:
			return false;
		case KeyEnter:
			return true;
		default:
			dodialog(id);
			break;
		}
	}
}

static struct widget_database_header : tree {

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