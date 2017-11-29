#include "amem.h"
#include "collections.h"
#include "crt.h"
#include "draw.h"
#include "draw_table.h"
#include "draw_textedit.h"
#include "io.h"
#include "xsref.h"

using namespace draw;
using namespace draw::controls;

xsfield widget_type[] = {
	BSREQ(widget, id, text_type),
	BSREQ(widget, flags, number_type),
	BSREQ(widget, label, text_type),
	BSREQ(widget, width, number_type),
	BSREQ(widget, childs, widget_type),
	{0}
};

static const int	table_padding = 4;
static char			search_text[32];
static unsigned		search_time;
const char*			table_sort_column_id;
bool				table_sort_by_mouse;

table::renderproc table::renders[] = {
	&renderno,
	&renderno, &renderno, &renderno, &renderno,
	&renderfield, &renderfield, &rendercheck, &renderfield, &renderimage,
	&linenumber,
};
static_assert(sizeof(table::renders) / sizeof(table::renders[0]) == LineNumber + 1, "Invalid table render procs");

static void setposition(rect& rc)
{
	rc.offset(table_padding, table_padding);
}

//	szprint(text, "%1i, %2i", ((point*)&i)->x, ((point*)&i)->y);
//	szprint(text, "%1i, %2i, %3i", ((color*)&i)->r, ((color*)&i)->g, ((color*)&i)->b);

unsigned table::add(bool run)
{
	if(no_change_count)
		return Disabled;
	if(no_change_max_count)// && pc->rows.count>= pc->rows.count_max)
		return Disabled;
	if(no_change_content)
		return Disabled;
	if(run)
	{
		auto p = rows.add();
		select(rows.indexof(p));
		invoke("change");
	}
	return 0;
}

unsigned table::addcopy(bool run)
{
	if(no_change_count)
		return Disabled;
	if(no_change_max_count)// && pc->rows.count>= pc->rows.count_max)
		return Disabled;
	if(no_change_content)
		return Disabled;
	if(run)
	{
		auto p = rows.add(rows.get(current));
		select(rows.indexof(p));
		invoke("change");
	}
	return 0;
}

unsigned table::copy(bool run)
{
	if(run)
	{
		char temp[4096]; temp[0] = 0;
		auto p = gettext(temp, rows.get(current), columns[current_column].id);
		if(p)
			clipboard::copy(p, zlen(p));
	}
	return 0;
}

unsigned table::remove(bool run)
{
	if(no_change_order)
		return Disabled;
	int i = rows.getcount();
	if(i == 0)
		return Disabled;
	if(run)
	{
		rows.remove(current);
		if(current >= i - 1)
			current--;
		correction();
		ensurevisible();
	}
	return 0;
}

unsigned table::change(bool run)
{
	if(no_change_content)
		return Disabled;
	if(columns.data[current_column].flags&ColumnReadOnly)
		return Disabled;
	if(!canedit(current, columns.data[current_column]))
		return Disabled;
	if(run)
		changing((void*)rows.get(current),
			columns.data[current_column].id,
			columns.data[current_column].flags);
	return 0;
}

unsigned table::movedown(bool run)
{
	if(no_change_order)
		return Disabled;
	int i = rows.getcount();
	if(i < 2 || current >= i - 1)
		return Disabled;
	if(run)
	{
		rows.swap(current, current + 1);
		current++;
		correction();
		ensurevisible();
	}
	return 0;
}

unsigned table::moveup(bool run)
{
	if(no_change_order)
		return Disabled;
	int i = rows.getcount();
	if(i < 2 || current == 0)
		return Disabled;
	if(run)
	{
		rows.swap(current, current - 1);
		current--;
		correction();
		ensurevisible();
	}
	return 0;
}

unsigned table::sortas(bool run)
{
	if(no_change_order)
		return Disabled;
	if(rows.getcount() < 2)
		return Disabled;
	if(run)
	{
		if(table_sort_by_mouse)
			sort(table_sort_column_id, 1);
		else
			sort(columns.data[current_column].id, 1);
		table_sort_by_mouse = false;
	}
	return 0;
}

unsigned table::sortds(bool run)
{
	if(no_change_order)
		return Disabled;
	if(rows.getcount() < 2)
		return Disabled;
	if(run)
	{
		if(table_sort_by_mouse)
			sort(table_sort_column_id, -1);
		else
			sort(columns.data[current_column].id, -1);
		table_sort_by_mouse = false;
	}
	return 0;
}

unsigned table::exportdata(bool run)
{
	char temp[260] = {0};
	char filter[2048]; io::plugin::getfilter(filter);
	if(run)
	{
		if(dialog::save(0, temp, filter))
			rows.write(temp, fields);
	}
	return 0;
}

unsigned table::importdata(bool run)
{
	if(no_change_order || no_change_content || no_change_count)
		return Disabled;
	if(run)
	{
		char temp[260] = {0};
		char filter[2048]; io::plugin::getfilter(filter);
		if(dialog::open(0, temp, filter))
			rows.read(temp, fields);
	}
	return 0;
}

unsigned table::setting(bool run)
{
	if(!use_setting)
		return Disabled;
	if(run)
	{
		adatc<widget, 64> data_columns;
		table table_columns(data_columns);
		table_columns.fields = widget_type;
		window dc(-1, -1, 400, 300, WFMinmax | WFResize, 0, "TableSetting");
		draw::control* controls[64] = {&table_columns, 0};
		setcaption("Настройки");
		const int dy = texth() + 8 + metrics::padding * 2;
		int current_control = 0;
		table_columns.addcol(WidgetField | ColumnSizeAuto | ColumnReadOnly, "label", "Название");
		table_columns.addcol(WidgetField, "width", "Ширина");
		table_columns.use_setting = false;
		table_columns.show_grid_lines = true;
		table_columns.focused = true;
		data_columns.clear();
		for(int i = 0; i < columns.count; i++)
		{
			data_columns.add(columns.data[i]);
		}
		tuning(controls);
		setfocus(0);
		while(true)
		{
			rect rc = {0, 0, getwidth(), getheight() - dy};
			rectf({0, 0, getwidth(), getheight()}, colors::form);
			rc.offset(metrics::padding * 2);
			draw::view(rc, controls, zlen(controls), current_control, false, 0, metrics::padding);
			rc.y1 = getheight() - dy - metrics::padding;
			rc.x2 = getwidth() - metrics::padding;
			rc.x1 = rc.x2 - 100; button(rc.x1, rc.y1, 100, KeyEscape, "Отменить");
			rc.x1 = rc.x1 - 100; button(rc.x1, rc.y1, 100, KeyEnter, "OK");
			int id = input();
			switch(id)
			{
			case 0:
			case KeyEscape:
				return false;
			case KeyEnter:
				for(unsigned i = 0; i < data_columns.count; i++)
					columns.data[i] = data_columns.data[i];
				return true;
			default:
				if(dodialog(id))
					break;
				table_columns.keyinput(id);
				break;
			}
		}
	}
	return 0;
}

table::table(collection& rows) : rows(rows), rowsimages(metrics::tree),
maximum_column(0), current_column(0),
no_change_max_count(false), no_change_count(false), no_change_order(false), no_change_content(false),
group_sort_up(false),
use_setting(true),
show_header(true), show_event_rows(false)
{
	id = "table";
	columns.data = 0;
	columns.count = 0;
}

static int column_total_width;

void table::background(rect& rc)
{
	// background
	list::background(rc);
	// drag&drop
	if(drag::active(id, DragColumn))
		columns.data[drag::value].width = imax(hot::mouse.x - drag::mouse.x - column_total_width, 8);
	// calculate size
	reposition(rc.width());
	// show header
	if(show_header)
	{
		header({rc.x1, rc.y1, rc.x2, rc.y1 + pixels_per_line});
		rc.y1 += pixels_per_line;
	}
}

void table::prerender()
{
	if(!rows.getcount())
		expand(0, 0);
	maximum = rows.getcount();
	validate(1, false);
	//
	maximum_width = 0;
	if(columns.data)
	{
		for(auto& e : columns)
		{
			if(e.flags&ColumnHide)
				continue;
			maximum_width += e.width;
		}
	}
}

void table::header(rect client)
{
	static int last_direction;
	draw::state push;
	setclip(client);
	rect rc = client;
	rc.x1 -= origin_width;
	bool mouse_hilite = areb(client);
	gradv(client, colors::form.lighten(), colors::form.darken());
	int w = 0;
	if(!columns.data)
		return;
	auto bc = getcolor(colors::border);
	for(auto& e : columns)
	{
		if(e.flags&ColumnHide)
			continue;
		w += e.width;
		rc.x2 = rc.x1 + w;
		if((e.flags&ColumnSizeMask) == ColumnSizeHide)
			continue;
		if(rc.x2 > client.x2)
			rc.x2 = client.x2;
		areas a = area(rc);
		if(mouse_hilite
			&& ((e.flags&ColumnSizeMask) == ColumnSizeNormal)
			&& hot::mouse.x >= rc.x2 - 3 && hot::mouse.x <= rc.x2 + 3)
		{
			hot::cursor = CursorLeftRight;
			a = AreaNormal;
			if(hot::pressed && hot::key == MouseLeft)
			{
				column_total_width = w - e.width;
				drag::begin(id, DragColumn);
				drag::mouse.x = hot::mouse.x - w;
				drag::value = columns.indexof(&e);
			}
		}
		switch(a)
		{
		case AreaNormal:
			break;
		case AreaHilited:
			gradv({rc.x1 + ((&e == columns.data) ? 0 : 1), rc.y1 + 1, rc.x2, rc.y2},
				colors::button.lighten(),
				colors::button.darken());
			break;
		case AreaHilitedPressed:
			gradv({rc.x1 + ((&e == columns.data) ? 0 : 1), rc.y1 + 1, rc.x2, rc.y2},
				colors::button.darken(),
				colors::button.lighten());
			break;
		default:
			break;
		}
		if(areb(rc) && hot::key == MouseLeft && !hot::pressed)
		{
			if(e.gettype() != LineNumber)
			{
				table_sort_column_id = e.id;
				table_sort_by_mouse = true;
				last_direction = (last_direction == -1) ? 1 : -1;
				if(last_direction == -1)
					invoke("sortas");
				else
					invoke("sortds");
			}
		}
		if(e.label)
		{
			auto type = e.gettype();
			if(type == WidgetCheck || type == WidgetImage)
				showlabel({rc.x1 + 4, rc.y1 + 4, rc.x2 - 8, rc.y2 - 4}, e.label, TextSingleLine | AlignCenterCenter);
			else
				showlabel({rc.x1 + 4, rc.y1 + 4, rc.x2 - 8, rc.y2 - 4}, e.label, TextSingleLine);
		}
		if(e.tips && areb(rc))
			tooltips(e.tips);
		line(rc.x2, rc.y1, rc.x2, rc.y2, bc);
		rc.x1 = rc.x2;
		w = 0;
	}
	if(show_border)
	{
		line(client.x1, client.y1, client.x1, client.y2, bc);
		line(client.x1, client.y1, client.x2, client.y1, bc);
		line(client.x2, client.y1, client.x2, client.y2, bc);
	}
	line(client.x1, client.y2 - 1, client.x2, client.y2 - 1, bc);
}

void table::showlabel(rect rc, const char* value, unsigned flags) const
{
	flags = flags & 0xFFFFFFF0;
	draw::state push;
	draw::setclip(rc);
	draw::text(rc, value, flags);
	if(draw::areb(rc))
	{
		int w1 = draw::textw(value);
		int w = rc.width();
		if(w1 > w)
		{
			rect rc1;
			rc1.x1 = rc.x1 - metrics::padding;
			rc1.y1 = rc.y1 - metrics::padding;
			rc1.x2 = rc.x1 + imin(w1, 320) + metrics::padding * 2;
			rc1.y2 = rc.y1 + draw::texth() + 4;
			tooltips(rc1, value);
		}
	}
}

void table::showtext(rect rc, const char* value, unsigned flags) const
{
	if(ischecked(flags))
	{
		if(value && flags&ColumnSmallHilite)
			draw::hilight({rc.x1, rc.y1, imin(rc.x2, rc.x1 + draw::textw(value) + 12), rc.y2}, flags);
		else
			draw::hilight(rc, flags);
	}
	setposition(rc);
	showlabel(rc, value, flags);
}

const char* table::gettext(char* result, void* data, const char* id) const
{
	return fields->getdata(result, id, data, false);
}

void table::renderno(rect rc, int index, unsigned flags, void* data, const widget& e) const
{
}

void table::rendercheck(rect rc, int index, unsigned flags, void* data, const widget& e) const
{
	if(ischecked(flags))
		draw::hilight(rc, flags);
	xsref r = {fields, data};
	auto ischecked = false;
	if(e.value)
		ischecked = (r.get(e.id) & e.value) != 0;
	else
		ischecked = r.get(e.id) != 0;
	setposition(rc);
	auto executed = false;
	if(areb(rc))
	{
		if(hot::key == MouseLeft && hot::pressed)
			executed = true;
	}
	if((flags&(Checked | Focused)) == (Checked | Focused) && hot::key == KeySpace)
		executed = true;
	if(executed)
	{
		if(ischecked)
		{
			if(e.value)
				r.set(e.id, r.get(e.id) & (~e.value));
			else
				r.set(e.id, 0);
			ischecked = false;
		}
		else
		{
			ischecked = true;
			if(e.value)
				r.set(e.id, e.value);
			else
				r.set(e.id, 1);
		}
	}
	clipart(rc.x1, rc.y1, rc.width(), ischecked ? Checked : 0, ":check");
}

void table::linenumber(rect rc, int index, unsigned flags, void* data, const widget& e) const
{
	char temp[32]; sznum(temp, index + 1);
	showtext(rc, temp, flags);
}

void table::renderfield(rect rc, int index, unsigned flags, void* data, const widget& e) const
{
	auto requisit = fields->find(e.id);
	if(!requisit)
		return;
	char temp[1024];
	auto p = gettext(temp, data, e.id);
	if(p)
		showtext(rc, p, flags);
}

void table::renderimage(rect rc, int index, unsigned flags, void* data, const widget& e) const
{
	if(rowsimages)
		return;
	auto requisit = fields->find(e.id);
	if(!requisit)
		return;
	auto value = requisit->get(data);
	if(value == -1)
		return;
	setposition(rc);
	draw::image(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2, rowsimages, value, 0);

}

void table::row(rect rc, int index)
{
	rect r1;
	if(show_event_rows && (index % 2) != 0)
		rectf({rc.x1 + 1, rc.y1, rc.x2, rc.y2}, colors::active.mix(colors::window, 16));
	r1 = rc;
	// Tree checkmark
	int level = getlevel(index);
	if(level)
	{
		int dy = rc.height() - 2;
		r1.x1 = rc.x1 + (level - 1) * dy;
		r1.x2 = r1.x1 + dy;
		treemark(r1, index, level);
		r1.x1 = r1.x2;
	}
	// Rest of rows
	int w = r1.x1 - rc.x1;
	int i = 0;
	bool first_row = true;
	if(!columns.data)
		return;
	void* data = rows.get(index);
	for(auto& e : columns)
	{
		if(e.flags&ColumnHide)
			continue;
		int t = e.flags&ColumnSizeMask;
		r1.x2 = r1.x1 + e.width;
		if(first_row && (t == ColumnSizeNormal || t == ColumnSizeAuto))
		{
			first_row = false;
			r1.x2 -= w;
		}
		if(r1.x1 >= rc.x2)
			break;
		unsigned flags = e.flags;
		if(focused)
			flags |= Focused;
		if(index == current && i == current_column)
		{
			flags |= Checked;
			hot::element = r1;
		}
		(this->*renders[e.gettype()])(r1, index, flags, data, e);
		if(show_grid_lines)
			line(r1.x2, r1.y1, r1.x2, r1.y2 - 1, colors::form);
		r1.x1 = r1.x2;
		i++;
	}
}

struct table_compare_data
{
	table*				parent;
	table::sortinfo*	orders;
};

static int compare(const void* p1, const void* p2, void* param)
{
	auto cd = (table_compare_data*)param;
	auto pc = cd->parent;
	int result = 0;
	for(auto d = cd->orders; *d; d++)
	{
		auto fd = pc->fields->find(d->id);
		if(!fd)
			continue;
		auto v1 = fd->get(fd->ptr(p1));
		auto v2 = fd->get(fd->ptr(p2));
		if(fd->type == text_type && v1 && v2)
			result = strcmp((char*)v1, (char*)v2);
		else
			result = v1 - v2;
		if(result != 0)
			return result*d->direction;
	}
	return 0;
}

static int compare_group_up(const void* p1, const void* p2, void* param)
{
	const table_compare_data* cd = (table_compare_data*)param;
	int g1 = cd->parent->isgroup(cd->parent->rows.indexof(p1)) ? 1 : 0;
	int g2 = cd->parent->isgroup(cd->parent->rows.indexof(p2)) ? 1 : 0;
	if(g1 == g2)
		return compare(p1, p2, param);
	return g2 - g1;
}

widget*	table::findcol(const char* id)
{
	for(auto& e : columns)
	{
		if(!e.id)
			continue;
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

void table::sort(table::sortinfo* psi, int i1, int i2)
{
	if(i2 == -1)
		i2 = maximum - 1;
	if(i2 <= 0)
		return;
	table_compare_data cd;
	cd.orders = psi;
	cd.parent = this;
	rows.sort(i1, i2, group_sort_up ? compare_group_up : compare, &cd);
}

void table::sort(const char* id, int direction, int i1, int i2)
{
	sortinfo si[2] = {0};
	si[0].id = id;
	si[0].direction = direction;
	sort(si, i1, i2);
}

bool table::canedit(int index, const widget& e) const
{
	switch(e.gettype())
	{
	case LineNumber:
	case WidgetImage:
		return false;
	default:
		return true;
	}
}

void table::validate(int direction, bool editable)
{
	maximum_column = columns.count;
	if(!maximum_column)
		return;
	while(true)
	{
		if(current_column < 0)
		{
			current_column = 0;
			direction = 1;
		}
		else if(current_column > maximum_column - 1)
		{
			current_column = maximum_column - 1;
			direction = -1;
		}
		auto& e = columns.data[current_column];
		if((!editable || (editable && e.gettype() != WidgetCheck))
			&& canedit(current, e)
			&& (e.flags&ColumnHide) == 0)
			return;
		current_column += direction;
	}
}

bool table::selecting(rect rc, int index, point mouse)
{
	rc.x1 -= origin_width;
	rect r1 = rc;
	int level = getlevel(index);
	if(level)
	{
		int dy = rc.height() - 2;
		r1.x1 = r1.x1 + (level - 1) * dy;
		r1.x2 = r1.x1 + dy;
		if(mouse.in(r1))
		{
			toggle(index);
			return false;
		}
		r1.x1 = r1.x2;
	}
	// Rest of rows
	int w = r1.x1 - rc.x1;
	int i = 0;
	bool first_row = true;
	if(!columns.data)
		return false;
	for(auto& e : columns)
	{
		if(e.flags&ColumnHide)
			continue;
		int t = e.flags&ColumnSizeMask;
		r1.x2 = r1.x1 + e.width;
		if(first_row && (t == ColumnSizeNormal || t == ColumnSizeAuto))
		{
			first_row = false;
			r1.x2 -= w;
		}
		if(r1.x1 >= rc.x2)
			break;
		if(mouse.in(r1))
		{
			if(!canedit(index, e))
				return false;
			if(e.gettype() == WidgetCheck)
				return false;
			current_column = columns.indexof(&e);
			break;
		}
		r1.x1 = r1.x2;
	}
	return true;
}
void table::treemark(rect rc, int index, int level) const
{
	if(!isgroup(index))
		return;
	color c1 = colors::text;
	int	count = maximum;
	bool isopen = (index < count - 1) ? (getlevel(index + 1) > level) : false;
	int x = rc.x1 + rc.width() / 2;
	int y = rc.y1 + rc.height() / 2 - 1;
	areas a = area(rc);
	if(a == AreaHilitedPressed)
		circlef(x, y, 6, colors::window.mix(colors::button, 196));
	circle(x, y, 6, c1);
	line(x - 4, y, x + 4, y, c1);
	if(!isopen)
		line(x, y - 4, x, y + 4, c1);
}

int table::find(const char* id, const char* value, int index)
{
	int i1 = index;
	int i2 = maximum;
	int sz = zlen(value);
	auto type = fields->find(id);
	if(!type)
		return -1;
	for(; i1 < i2; i1++)
	{
		char temp[4096];
		auto pv = fields->getdata(temp, id, rows.get(i1), false);
		if(!pv)
			continue;
		if(szcmpi(pv, value, sz) == 0)
			return i1;
	}
	return -1;
}

bool table::changing(void* object, const char* id, unsigned flags)
{
	char temp[4196]; temp[0] = 0;
	if(!fields->getdata(temp, id, object, true))
		return false;
	draw::rectf(hot::element, colors::window);
	if(show_grid_lines)
	{
		line(hot::element.x1, hot::element.y1, hot::element.x1, hot::element.y2, colors::form);
		line(hot::element.x1, hot::element.y2, hot::element.x2, hot::element.y2, colors::form);
	}
	textedit te(temp, sizeof(temp), true);
	te.align = flags;
	auto result = te.editing(hot::element);
	if(result)
		fields->setdata(temp, id, object);
	// Some keys must be handled by this control
	switch(hot::key)
	{
	case KeyDown:
	case KeyUp:
		draw::execute(hot::key, 0);
		break;
	}
	return result;
}

void table::clear()
{
	rows.clear();
	current = 0;
}

void table::keyleft(int id)
{
	auto i = current_column;
	if(current_column)
	{
		current_column--;
		validate(-1, false);
	}
	if(i == current_column)
	{
		if(isopen(current))
			toggle(current);
		else
			current = getparent(current);
		ensurevisible();
		validate(-1, false);
	}
}

void table::keyright(int id)
{
	if(isgroup(current) && !isopen(current))
		toggle(current);
	else if(current_column < maximum_column - 1)
	{
		current_column++;
		validate(1, false);
	}
}

void table::inputsymbol(int id, int symbol)
{
	if(!symbol || symbol < 0x20)
		return;
	auto time_clock = clock();
	if(!search_time || (time_clock - search_time) > (2 * 1000))
		search_text[0] = 0;
	search_time = time_clock;
	char* p = zend(search_text);
	szput(&p, hot::param);
	p[0] = 0;
	int i1 = find(columns.data[current_column].id, search_text, current);
	if(i1 != -1)
	{
		current = i1;
		correction();
		ensurevisible();
	}
}

widget& table::addcol(unsigned flags, const char* id, const char* label, int width)
{
	auto p = columns.addu(id);
	memset(p, 0, sizeof(p[0]));
	p->id = id;
	p->label = label;
	p->flags = flags;
	p->width = width;
	switch(p->gettype())
	{
	case LineNumber:
		if(p->getflags() == 0)
			p->flags |= AlignRightCenter;
		break;
	case WidgetCheck:
	case WidgetImage:
		p->flags |= ColumnHideTitleText;
		break;
	}
	if(p->width == -1)
	{
		switch(p->gettype())
		{
		case LineNumber:
			p->width = 50;
			break;
		case WidgetCheck:
		case WidgetImage:
			p->width = 22;
			p->flags |= ColumnSizeFixed;
			break;
		default:
			if(fields && (p->flags&AlignMask) == 0)
			{
				auto requisit = fields->find(p->id);
				if(requisit)
				{
					if(requisit->type == number_type)
						p->flags |= AlignRightCenter;
				}
			}
			p->width = 150;
			break;
		}
		p->flags |= TextSingleLine;
	}
	return *p;
}

int table::totalwidth() const
{
	int result = 0;
	if(!columns.data)
		return 0;
	for(auto& e : columns)
		result += e.width;
	return result;
}

void table::reposition(int w1)
{
	int w2 = 0;
	int c2 = 0;
	const int min_width = 8;
	if(!columns.data)
		return;
	for(auto& e : columns)
	{
		if((e.flags&ColumnSizeMask) == ColumnSizeAuto)
		{
			w2 += min_width;
			c2++;
		}
		else
		{
			// Check minimal width
			switch(e.gettype())
			{
			case WidgetCheck:
			case WidgetImage:
				if(e.width < 18)
					e.width = 18;
				break;
			}
			w2 += e.width;
		}
	}
	// calculate auto sized width
	if(w2 < w1 && c2)
	{
		int d1 = w1 - w2;
		int d2 = d1 / c2;
		for(auto& e : columns)
		{
			if((e.flags&ColumnSizeMask) == ColumnSizeAuto)
			{
				if(d2 < d1)
					e.width = min_width + d2;
				else
				{
					e.width = min_width + d1;
					break;
				}
			}
		}
	}
}

void table::tuning(draw::control** data)
{
}

void table::contextmenu()
{
	menu e;
	if(!no_change_content && !no_change_order)
	{
		e.add("add", this);
		e.add("addcopy", this);
	}
	if(!no_change_content)
		e.add("change", this);
	e.addseparator();
	if(!no_change_content && !no_change_order)
		e.add("cut", this);
	e.add("copy", this);
	if(!no_change_content && !no_change_order)
		e.add("paste", this);
	e.add("delete", this);
	e.addseparator();
	if(!no_change_order)
	{
		e.add("moveup", this);
		e.add("movedown", this);
		e.addseparator();
		e.add("sortas", this);
		e.add("sortds", this);
		e.addseparator();
	}
	e.addseparator();
	e.add("import", this);
	e.add("export", this);
	e.addseparator();
	if(use_setting)
		e.add("setting", this);
	auto result = (control::command*)e.choose(hot::mouse.x, hot::mouse.y);
	if(result)
		(this->*result->type)(true);
}

control::command table::commands[] = {
	CONTROL_ICN(add, "Добавить", F8, 0),
	CONTROL_ICN(addcopy, "Скопировать", F9, 9),
	CONTROL_ICN(change, "Изменить", F2, 10),
	CONTROL_ICN(copy, "Копировать", Ctrl + Alpha + 'C', 4),
	CONTROL_ICN(remove, "Удалить", KeyDelete, 19),
	CONTROL_ICN(moveup, "Переместить вверх", 0, 21),
	CONTROL_ICN(movedown, "Переместить вниз", 0, 22),
	CONTROL_ICN(sortas, "Сортировать по возрастанию", 0, 11),
	CONTROL_ICN(sortds, "Сортировать по убыванию", 0, 12),
	CONTROL_ICN(exportdata, "Экспортировать данные", 0, 2),
	CONTROL_ICN(importdata, "Импортировать данные", 0, 1),
	CONTROL_ICN(setting, "Настройки", 0, 16),
	{0}
};