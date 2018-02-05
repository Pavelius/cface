#include "command.h"
#include "crt.h"
#include "collections.h"
#include "database.h"
#include "draw_tree.h"
#include "settings.h"
#include "bsreq.h"

using namespace	draw;
using namespace	draw::controls;

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

	widget_database_header() {
		show_toolbar = false;
		no_change_content = true;
		no_change_count = true;
	}

} database_header_control;