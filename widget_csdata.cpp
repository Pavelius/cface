#include "csdata.h"
#include "collections.h"
#include "command.h"
#include "crt.h"
#include "draw.h"
#include "draw_textedit.h"
#include "draw_tree.h"
#include "settings.h"
#include "widget.h"

using namespace	draw;
using namespace	csdata;

extern adat<typeinfo, 256> typeinfo_data;
extern adat<requisit, 8196> requisit_data;

static struct widget_csdata_header : controls::tree {

	void initialize() {
		addcol(WidgetImage, "image", "Изображение");
		addcol(WidgetField | ColumnSizeAuto | ColumnSmallHilite, "name", "Наименование");
		fields = typeinfo_type;
	}

	const char* getid() const override {
		return "headers";
	}

	char* getname(char* result) const override {
		zcpy(result, "База данных");
		return result;
	}

	bool hasrows(typeinfo* row) {
		for(auto& e : typeinfo_data) {
			if(e.parent == row)
				return true;
		}
		return false;
	}

	void addrows(const typeinfo* row) {
		for(auto& e : typeinfo_data) {
			if(e.parent == row)
				tree::addrow((unsigned)&e, hasrows(&e) ? TIGroup : 0, 1, 0);
		}
		for(auto& e : requisit_data) {
			if(e.parent == row)
				tree::addrow((unsigned)&e, 0, 2, 0);
		}
	}

	void expanding() override {
		if(level == 0) {
			tree::addrow((unsigned)type_root, TIGroup, 3, 0);
			tree::addrow((unsigned)enum_root, TIGroup, 3, 0);
		} else {
			auto pe = (element*)rows.get(index);
			addrows((typeinfo*)pe->param);
		}
	}

	int getindex(const void* value) {
		return findbyparam((int)value);
	}

	void update(int parent, void* current_item) {
		if(parent != -1)
			expand(parent, getlevel(parent));
		else
			expand(0, 0);
		if(current_item) {
			current = getindex(current_item);
			ensurevisible();
		}
	}

	unsigned add(bool run) override {
		if(run) {
			auto p = (typeinfo*)bsdata::find(typeinfo_type)->add();
			p->name = "Новый тип";
			update(getparent(current), p);
			invoke("change");
		}
		return 0;
	}

	widget_csdata_header() {
		show_header = false;
		show_toolbar = false;
		show_border = true;
		element_param_have_row = true;
	}

} header_control;

static control::plugin plugin(header_control);

void initialize_database_view() {
	header_control.initialize();
}