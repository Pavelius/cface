#include "command.h"
#include "collections.h"
#include "crt.h"
#include "draw.h"
#include "draw_table.h"
#include "draw_textedit.h"
#include "io_plugin.h"
#include "settings.h"
#include "bsdata.h"

using namespace	draw;
using namespace	draw::controls;

bool				metrics::show::padding;
static int			current_tab;
static settings*	current_header;
static control*		active_workspace_tab;

static struct dock {
	const char*	name;
	const char*	id;
} dock_data[DockWorkspace + 1] = {
	{"������������ �����", "dock_left"},
	{"������������ ����� � �����", "dock_left_bottom"},
	{"������������ ������", "dock_right"},
	{"������������ ������ � �����", "dock_right_bottom"},
	{"������������ �����", "dock_bottom"},
	{"�� ������� �����", "dock_workspace"}
};
getstr_enum(dock)
bsreq dock_type[] = {
	BSREQ(dock, name, text_type),
	BSREQ(dock, id, text_type),
	{0}
};
BSMETA(dock)

static const char* gettabname(char* temp, void* p) {
	return ((settings*)p)->name;
}

static void callback_settab() {
	current_tab = hot::param;
}

static int compare_settings(const void* p1, const void* p2) {
	const settings* e1 = *((settings**)p1);
	const settings* e2 = *((settings**)p2);
	if(e1->priority != e2->priority)
		return e1->priority - e2->priority;
	return strcmp(e1->name, e2->name);
}

static void getsiblings(settings** result, unsigned maximum_count, settings* parent) {
	settings** ps = result;
	settings** pe = result + maximum_count - 1;
	settings* tabs = parent->child();
	if(tabs) {
		for(settings* p = tabs; p; p = p->next) {
			if(p->e_visible && !p->e_visible(*p))
				continue;
			if(ps < pe)
				*ps++ = p;
		}
	}
	*ps = 0;
	qsort(result, zlen(result), sizeof(result[0]), compare_settings);
}

static void callback_button() {
	auto p = (settings*)hot::param;
	if(p->e_execute)
		p->e_execute();
}

static void callback_bool() {
	auto p = (settings*)hot::param;
	*((bool*)p->data) = !*((bool*)p->data);
}

static void callback_radio() {
	auto p = (settings*)hot::param;
	*((int*)p->data) = p->value;
}

static void callback_up() {
	auto p = (settings*)hot::param;
	(*((int*)p->data))--;
}

static void callback_down() {
	auto p = (settings*)hot::param;
	(*((int*)p->data))++;
}

static void callback_choose_folder() {
	char temp[260]; temp[0] = 0;
	auto p = (settings*)hot::param;
	auto v = *((const char**)p->data);
	if(v)
		zcpy(temp, v);
	if(draw::dialog::folder("������� �����", temp)) {
		if(temp[0])
			*((const char**)p->data) = szdup(temp);
		else
			*((const char**)p->data) = 0;
	}
}

static void callback_choose_color() {
	auto p = (settings*)hot::param;
	draw::dialog::color(*((color*)p->data));
}

static void callback_edit() {
	char temp[4196]; temp[0] = 0;
	auto p = (settings*)hot::param;
	switch(p->type) {
	case settings::TextPtr:
	case settings::UrlFolderPtr:
		if(*((const char**)p->data))
			zcpy(temp, *((const char**)p->data), sizeof(temp) - 1);
		break;
	case settings::Int:
		sznum(temp, *((int*)p->data));
		break;
	}
	textedit te(temp, sizeof(temp), true);
	if(te.editing(hot::element)) {
		switch(p->type) {
		case settings::TextPtr:
		case settings::UrlFolderPtr:
			if(temp[0])
				*((const char**)p->data) = szdup(temp);
			else
				*((const char**)p->data) = 0;
			break;
		case settings::Int:
			*((int*)p->data) = sz2num(temp);
			break;
		}
	}
}

static struct widget_settings_header : list {
	settings* rows[128];

	void initialize() {
		getsiblings(rows, sizeof(rows) / sizeof(rows[0]), &settings::root);
		maximum = zlen(rows);
		if(current >= maximum - 1)
			current = maximum;
		if(current < 0)
			current = 0;
	}

	void row(rect rc, int index) override {
		list::row({rc.x1 + 1, rc.y1 + 1, rc.x2 - 1, rc.y2}, index);
		textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 8, rows[index]->name);
	}

	settings* getcurrent() {
		return rows[current];
	}

} setting_header;

static struct widget_control_viewer : tableref {

	void initialize() {
		static bsreq control_type[] = {
			BSREQ(control, dock, dock_type),
			BSREQ(control, disabled, number_type),
			BSREQ(control, focused, number_type),
			BSREQ(control, show_toolbar, number_type),
			BSREQ(control, show_background, number_type),
			{0},
		};
		show_toolbar = false;
		no_change_order = true;
		no_change_count = true;
		fields = control_type;
		addcol(WidgetLabel, "name", "������������");
		addcol(WidgetField, "dock", "������������");
		for(auto p = plugin::first; p; p = p->next)
			addelement(&p->element);
	}

	const char*	gettext(char* result, void* data, const char* id) const override {
		auto p = (control*)data;
		if(strcmp(id, "name") == 0)
			return p->getname(result);
		else if(strcmp(id, "description") == 0)
			return p->getdescription(result);
		return table::gettext(result, data, id);
	}

	widget_control_viewer() {
	}

} control_viewer;

static struct widget_settings : control {

	int header_width;

	static const char* getname(char* temp, settings& e) {
		zcpy(temp, e.name);
		szupper(temp);
		return temp;
	}

	char* getname(char* temp) const override {
		zcpy(temp, "���������");
		return temp;
	}

	static int buttonc(int x, int y, int width, int id, unsigned flags, color value, const char* tips, void(*callback)()) {
		char temp[128]; szprint(temp, "%1i, %2i, %3i", value.r, value.g, value.b);
		setposition(x, y, width);
		struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
		draw::focusing(id, flags, rc);
		if(buttonh({x, y, x + width, rc.y2},
			ischecked(flags), isfocused(flags), isdisabled(flags), true, value,
			temp, KeyEnter, false, tips)) {
			draw::execute(callback);
			hot::param = id;
		}
		return rc.height() + metrics::padding * 2;
	}

	static int element(int x, int y, int width, unsigned flags, settings& e) {
		const auto title = 160;
		settings* pc;
		char temp[512]; temp[0] = 0;
		if(e.e_visible && !e.e_visible(e))
			return 0;
		int y1 = y;
		switch(e.type) {
		case settings::Radio:
			y += radio(x, y, width, (int)&e, flags | ((*((int*)e.data) == e.value) ? Checked : 0),
				getname(temp, e), 0, callback_radio);
			break;
		case settings::Bool:
			y += checkbox(x, y, width, (int)&e, flags | (*((bool*)e.data) ? Checked : 0),
				getname(temp, e), 0, callback_bool);
			break;
		case settings::Int:
			if(e.value) // ���� ��������
			{
				auto w = (getdigitscount(e.value) + 1)*textw("0") + metrics::padding * 2 + 19;
				if(title + w < width)
					width = title + w;
			}
			sznum(temp, *((int*)e.data));
			y += field(x, y, width, (int)&e, flags, temp, 0, e.name, title, callback_edit, 0, 0, callback_up, callback_down);
			break;
		case settings::Color:
			titletext(x, y, width, flags, e.name, title);
			y += buttonc(x, y, width, (int)&e, flags, *((color*)e.data), 0, callback_choose_color);
			break;
		case settings::Button:
			y += button(x, y, width, (int)&e, flags, getname(temp, e), 0, callback_button);
			break;
		case settings::TextPtr:
			y += field(x, y, width, (int)&e, flags, *((const char**)e.data), 0, e.name, title, callback_edit);
			break;
		case settings::UrlFolderPtr:
			y += field(x, y, width, (int)&e, flags, *((const char**)e.data), 0, e.name, title, callback_edit, 0, callback_choose_folder);
			break;
		case settings::Control:
			break;
		case settings::Group:
			pc = e.child();
			if(!pc)
				return 0;
			if(true) {
				auto y2 = y;
				auto height = draw::texth() + metrics::padding * 2;
				y += height;
				for(; pc; pc = pc->next)
					y += element(x, y, width, flags, *pc);
				if(e.name) {
					color c1 = colors::border.mix(colors::window, 128);
					color c2 = c1.darken();
					gradv({x, y2, x + width, y2 + height}, c1, c2);
					fore = colors::text.mix(c1, 96);
					text(x + (width - textw(e.name)) / 2, y2 + metrics::padding, e.name);
					rectb({x, y2, x + width, y + metrics::padding}, colors::border);
				}
				y += metrics::padding * 2;
			}
			break;
		}
		return y - y1;
	}

	void redraw(rect rc) {
		draw::state push;
		settings* tabs[128];
		fore = colors::text;
		splitv(rc.x1, rc.y1, header_width, rc.height(), 1, 6, 64, 282);
		setting_header.show_border = metrics::show::padding;
		setting_header.viewf({rc.x1, rc.y1, rc.x1 + header_width, rc.y2}, false);
		rc.x1 += header_width + 6;
		auto top = setting_header.getcurrent();
		// ��� ��������� �������� ���������
		if(top != current_header) {
			current_header = top;
			current_tab = -1;
		}
		if(!top)
			return;
		getsiblings(tabs, sizeof(tabs) / sizeof(tabs[0]), top);
		if(tabs[0]) {
			// ������� �������������� ������
			if(current_tab == -1)
				current_tab = 0;
			int h1 = 28;
			// �������� ��������
			auto hilited = -1;
			if(draw::tabs({rc.x1, rc.y1, rc.x2, rc.y1 + h1 + 1}, false, false,
				(void**)tabs, 0, zlen(tabs), current_tab, &hilited,
				gettabname)) {
				draw::execute(callback_settab);
				hot::param = hilited;
			}
			line(rc.x1, rc.y1 + h1, rc.x2, rc.y1 + h1, colors::border);
			rc.y1 += h1 + metrics::padding * 2;
			rc.x1 += metrics::padding;
			rc.x2 -= metrics::padding;
			// ������� ������� ��������
			if(current_tab != -1) {
				int w4 = rc.width();
				int w3 = imin(w4, 640);
				auto p1 = tabs[current_tab];
				switch(p1->type) {
				case settings::Control:
					((control*)p1->data)->viewf(rc, false);
					break;
				default:
					for(auto p = p1->child(); p; p = p->next)
						rc.y1 += element(rc.x1, rc.y1, w3, 0, *p);
					break;
				}
			}
		}
	}

	widget_settings() : header_width(160) {
		show_background = false;
		show_border = false;
	}

} widget_settings_control;

static struct widget_application : control {

	static command	commands[];
	control*		hotcontrols[48];

	char* getname(char* temp) const override {
		zcpy(temp, "�������");
		return temp;
	}

	const command* getcommands() const override {
		return commands;
	}

	static const char* getcontrolname(char* temp, void* p) {
		return ((control*)p)->getname(temp);
	}

	unsigned create(bool run) {
		if(run) {

		}
		return Disabled;
	}

	unsigned load(bool run) {
		if(run) {

		}
		return 0;
	}

	unsigned save(bool run) {
		if(run) {

		}
		return 0;
	}

	static void workspace(rect rc, bool allow_multiply_window) {
		control* p1[64];
		int c1 = getdocked(p1, sizeof(p1) / sizeof(p1[0]), DockWorkspace);
		//int c2 = c1 + dialog::select(p1 + c1);
		if(c1 == 1 && !allow_multiply_window) {
			active_workspace_tab = p1[0];
			active_workspace_tab->view(rc, false);
		} else if(c1) {
			auto last_active_workspace_tab = active_workspace_tab;
			auto current_select = zfind(p1, active_workspace_tab);
			if(current_select == -1)
				current_select = 0;
			active_workspace_tab = p1[current_select];
			auto ec = active_workspace_tab;
			const int dy = draw::texth() + 8;
			rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
			auto result = draw::tabs(rct, true, false, (void**)p1, 0, c1,
				current_select, &current_select, getcontrolname, 0, {2, 0, 2, 0});
			//		if(c2 > c1)
			//		{
			//			rct.x1 += draw::tabs(rct, TabsControl, HideBackground, (void**)p1, c1, c2,
			//				current_select, &current_select, get_control_name, get_control_info, 2);
			//		}
			//		if(getcommand() == TabsControl)
			//		{
			//			if(current_select != -1)
			//				active_workspace_tab = p1[current_select];
			//		}
			//		else if(getcommand() == TabsCloseControl)
			//		{
			//			if(current_select != -1)
			//				close_workspace_tab = p1[current_select];
			//		}
			//		if(active_workspace_tab != last_active_workspace_tab)
			//		{
			//			if(active_workspace_tab)
			//				active_workspace_tab->execute(Update, true);
			//		}
			rc.y1 += dy;
			unsigned flags = ec->focused ? Focused : 0;
			if(ec->disabled)
				flags |= Disabled;
			draw::focusing((int)ec, flags, rc);
			ec->focused = isfocused(flags);
			ec->view(rc, false);
		}
	}

	void redraw(rect rc) {
		draw::dockbar(rc);
		workspace(rc, true);
	}

	widget_application() {
		show_background = false;
		show_border = false;
		memset(hotcontrols, 0, sizeof(hotcontrols));
	}

} widget_application_control;

control::command widget_application::commands[] = {
	CONTROL_ICN(create, "������� ����", Ctrl + Alpha + 'N', 0),
	CONTROL_ICN(load, "������� ����", Ctrl + Alpha + 'O', 1),
	CONTROL_ICN(save, "��������� ����", Ctrl+Alpha+'S', 2),
	{0}
};

static void setting_appearance_general_metrics() {
	settings& e1 = settings::root.gr("������� ����").gr("�����").gr("�������");
	e1.add("�������", metrics::padding);
	e1.add("������ �������", metrics::scroll);
}

static void setting_appearance_forms() {
	settings& e2 = settings::root.gr("�����").gr("�����");
	e2.add("���������� ������� ����", set_light_theme);
	e2.add("���������� ������ ����", set_dark_theme);
	settings& e3 = settings::root.gr("�����").gr("�����");
	e3.add("���� ������", colors::text);
	e3.add("���� ����", colors::window);
	e3.add("���� �����", colors::form);
	e3.add("���� �������", colors::border);
	e3.add("�������� ����", colors::active);
	e3.add("���� ������", colors::button);
	e3.add("���� ��������������", colors::edit);
	e3.add("���� ��������", colors::tabs::back);
	e3.add("���� ������ ��������", colors::tabs::text);
	e3.add("���� ���������", colors::tips::back);
	e3.add("���� ������ ���������", colors::tips::text);
}

static void setting_appearance_general_view() {
	settings& e1 = settings::root.gr("������� ����").gr("�����").gr("���");
	e1.add("���������� ������ �������", metrics::show::statusbar);
	e1.add("���������� ����� ������ ���������", metrics::show::left);
	e1.add("���������� ������ ������ ���������", metrics::show::right);
	e1.add("���������� ������ ������ ���������", metrics::show::bottom);
	e1.add("������� �� ������� ����", metrics::show::padding);
	//e1.add("������������ ����������� ��� �������� �����", sys_optimize_mouse_move);
}

static void setting_appearance_controls() {
	control_viewer.initialize();
	if(!control_viewer.rows.getcount())
		return;
	settings& e1 = settings::root.gr("����������").add("��������", control_viewer);
}

command* command_settings_initialize;

static void initialize_settings() {
	setting_appearance_general_metrics();
	setting_appearance_forms();
	setting_appearance_general_view();
	setting_appearance_controls();
	// Initialize other plugin settings
	command_settings_initialize->execute();
	// Make header
	setting_header.initialize();
}

static control* layouts[] = {&widget_application_control, &widget_settings_control};

static const char* get_control_name(char* result, void* object) {
	return ((control*)object)->getname(result);
}

static const char* get_control_status(char* result, void* object) {
	char temp[260];
	szprint(result, "����������� ��� �� '%1'", ((control*)object)->getname(temp));
	return result;
}

COMMAND(app_initialize) {
	initialize_settings();
}

int draw::application(const char* title) {
	draw::window dc(-1, -1, 600, 400, WFResize | WFMinmax, 32, "application");
	if(title)
		draw::setcaption(title);
	auto current_tab = 0;
	while(true) {
		auto pc = layouts[current_tab];
		auto commands = pc->getcommands();
		draw::fore = colors::text;
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form);
		rc.y2 -= draw::statusbardraw();
		rect rt = rc;
		rt.y2 = rt.y1 + (metrics::toolbar ? metrics::toolbar->get(0).sy + 4*2 : 24);
		sheetline(rt, true);
		rc.y1 += rt.height();
		rt.x1 += 2; rt.y1 += 1; rt.y2 -= 3; rt.x2 -= 2;
		if(metrics::show::padding)
			rc.offset(metrics::padding);
		pc->enablefocus();
		pc->background(rc);
		pc->prerender();
		pc->enablemouse(rc);
		pc->nonclient(rc);
		pc->toolbar(rt.x1, rt.y1, rt.width(), commands);
		auto hilite_tab = 0;
		auto reaction = draw::tabs(rt, false, true, (void**)layouts, 0,
			sizeof(layouts) / sizeof(layouts[0]), current_tab, &hilite_tab, get_control_name, get_control_status,
			{0, metrics::padding, 0, metrics::padding});
		if(reaction == 1 && hilite_tab != current_tab)
			current_tab = hilite_tab;
		auto id = draw::input();
		if(!id)
			return 0;
		if(control::dodialog(id))
			continue;
		switch(id) {
		case F2: metrics::show::bottom = !metrics::show::bottom; break;
		case Alt + F2: metrics::show::left = !metrics::show::left; break;
		case Ctrl + F2: metrics::show::right = !metrics::show::right; break;
		}
	}
}

static struct settings_settings_strategy : io::strategy {

	void write(io::writer& file, settings& e) {
		switch(e.type) {
		case settings::Group:
			file.open(e.identifier);
			for(settings* p = (settings*)e.data; p; p = p->next)
				write(file, *p);
			file.close(e.identifier);
			break;
		case settings::Bool:
			file.set(e.identifier, *((bool*)e.data));
			break;
		case settings::Int:
		case settings::Color:
			file.set(e.identifier, *((int*)e.data));
			break;
		case settings::Radio:
			if(*((int*)e.data) == e.value)
				file.set(e.identifier, 1);
			break;
		case settings::TextPtr:
		case settings::UrlFolderPtr:
			file.set(e.identifier, *((const char**)e.data));
			break;
		}
	}

	void write(io::writer& file, void* param) override {
		write(file, settings::root);
	}

	settings* find(io::reader::node& n) {
		auto result = (settings*)settings::root.data;
		if(n.parent && strcmp(n.parent->name, "Root")!=0) {
			result = find(*n.parent);
			if(!result)
				return 0;
			if(result->type != settings::Group)
				return 0;
			result = (settings*)result->data;
		}
		return result->find(szdup(n.name));
	}

	int getnum(const char* value) const {
		return sz2num(value);
	}

	void set(io::reader::node& n, const char* value) override {
		auto e = find(n);
		if(!e)
			return;
		switch(e->type) {
		case settings::Int:
		case settings::Color:
			*((int*)e->data) = getnum(value);
			break;
		case settings::Bool:
			*((bool*)e->data) = (getnum(value) ? true : false);
			break;
		case settings::Radio:
			*((int*)e->data) = e->value;
			break;
		case settings::TextPtr:
		case settings::UrlFolderPtr:
			*((const char**)e->data) = szdup(value);
			break;
		}
	}

	settings_settings_strategy() : strategy("settings", "settings") {}

} settings_settings_strategy_instance;

static struct controls_settings_strategy : io::strategy {

	void write(io::writer& file, void* param) override {
		for(auto pp = control::plugin::first; pp; pp = pp->next) {
			auto& e = pp->element;
			auto id = e.getid();
			if(!id || id[0] == 0)
				continue;
			file.open(id);
			file.set("Docking", e.dock);
			file.set("Disabled", e.disabled ? "true" : "false");
			file.close(id);
		}
	}

	settings* find(io::reader::node& n) {
		settings* result = (settings*)settings::root.data;
		if(n.parent && szcmpi(n.parent->name, "Root") != 0) {
			result = find(*n.parent);
			if(!result)
				return 0;
			if(result->type != settings::Group)
				return 0;
			result = (settings*)result->data;
		}
		return result->find(szdup(n.name));
	}

	bool istrue(const char* value) const {
		return value[0] == 't'
			&& value[1] == 'r'
			&& value[2] == 'u'
			&& value[3] == 'e'
			&& value[4] == 0;
	}

	void set(io::reader::node& n, const char* value) override {
		if(!n.parent || !n.parent->parent)
			return;
		auto e = control::plugin::find(n.parent->name);
		if(!e)
			return;
		if(szcmpi(n.name, "Disabled") == 0)
			e->element.disabled = istrue(value);
		else if(szcmpi(n.name, "Docking") == 0)
			e->element.dock = (dock_s)sz2num(value);
	}

	controls_settings_strategy() : strategy("controls", "settings") {}

} controls_settings_strategy_instance;