#include "command.h"
#include "crt.h"
#include "collections.h"
#include "draw.h"
#include "draw_tree.h"
#include "io.h"
#include "settings.h"
#include "xsfield.h"

using namespace	draw;
using namespace	draw::controls;

static const char*	project_path;

static struct widget_navigator : tree {

	const char* filter;

	struct element : tree::element {
		char	name[32];
	};

	static int compare(const void* p1, const void* p2, void* param) {
		element* m1 = (element*)p1;
		element* m2 = (element*)p2;
		return strcmp(szfname(m1->name), szfname(m2->name));
	}

	void initialize() {
		addcol(WidgetImage | ColumnSizeFixed, "image", "Изображение");
		addcol(WidgetField | ColumnSizeAuto | ColumnSmallHilite, "name", "Наименование");
	}

	const char* getid() const override {
		return "navigator";
	}

	char* getname(char* result) const override {
		zcpy(result, "Навигатор");
		return result;
	}

	void addurl(char* result, int row) {
		if(row == -1)
			return;
		addurl(result, getparent(row));
		element* pe = (element*)amem::get(row);
		if(!pe->name[0])
			return;
		zcat(result, "/");
		zcat(result, pe->name);
	}

	void expanding() override {
		element row;
		char temp[260]; temp[0] = 0;
		if(project_path)
			zcpy(temp, project_path);
		if(level)
			addurl(temp, index);
		for(io::file::find fe(temp); fe; fe.next()) {
			const char* fn = fe.name();
			if(fn[0] == '.' || fn[0] == '$' || fn[0] == ' ')
				continue;
			const char* e = szext(fn);
			if(e && filter && !szpmatch(fn, filter))
				continue;
			szfnamewe(temp, fn);
			if(zlen(temp) >= int(sizeof(element::name) - 1))
				continue;
			memset(&row, 0, sizeof(element));
			row.flags = e ? 0 : TIGroup;
			zcpy(row.name, temp);
			addrow(row);
		}
	}

	widget_navigator() : tree(sizeof(element)) {
		static xsfield element_type[] = {
			BSREQ(element, name, text_type),
			BSREQ(element, image, number_type),
			{0}
		};
		fields = element_type;
		show_toolbar = false;
		no_change_content = true;
		no_change_count = true;
		show_header = false;
		dock = DockLeft;
		filter = "*.bns";
		metrics::show::left = true;
	}

} instance;
static control::plugin plugin(instance);

void update_navigator() {
}

COMMAND(settings_initialize) {
	settings& e1 = settings::root.gr("Компилятор").gr("Основные").gr("Директории");
	e1.add("Проекты", project_path, settings::UrlFolderPtr);
	instance.initialize();
}