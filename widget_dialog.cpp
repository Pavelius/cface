#include "draw.h"
#include "command.h"
#include "crt.h"
#include "io.h"
#include "widget_settings.h"
#include "widget_tree.h"

namespace draw
{
	struct layout
	{
		const char*			name[2];
		int					priority;
		layout*				next;
		static layout*		first;
		static int			toolheight;
		//
		layout();
		static void			background(rect& rc, rect& rt, unsigned& flags);
		static bool			ischanging();
		virtual int			redraw() = 0;
		static void			setlayout(const char* name);
	};
}

using namespace draw;
//
const int				Tabs = 8096; // Standart tabs
const int				FirstControl = Tabs + 1;
const int				LastControl = FirstControl + 64;
//
static settings*		elements[LastControl - FirstControl + 1];
static int				current_tab = -1;
static settings*		current_header = 0;
static int				header_width = 200;
static const char*		layout_name = "Main";
static const char*		layout_param;
int						layout::toolheight = 0;
layout*					layout::first;
layout*					layouts[128];
static aref<draw::control*>	objects;
static draw::control*	close_workspace_tab;
draw::control*			active_workspace_tab;
static bool				allow_multiply_window;
static bool				show_file_extension;
static bool				first_letter_to_uppercase;

extern bool				sys_optimize_mouse_move;
extern rect				sys_static_area;

layout::layout() : priority(priority), next(0)
{
	if(!first)
		first = this;
	else
		seqlast(first)->next = this;
}

void layout::setlayout(const char* name)
{
	layout_param = name;
}

bool layout::ischanging()
{
	if(layout_param && layout_name != layout_param)
	{
		layout_name = layout_param;
		return true;
	}
	layout_param = 0;
	return false;
}

static char* get_control_name(char* result, void* p)
{
	result[0] = 0;
	auto po = static_cast<control*>(p);
	po->getname(result);
	auto xr = po->getr();
	if(!show_file_extension)
		szfnamewe(result, result);
	if(first_letter_to_uppercase)
		szupper(result, 1);
	if(po->ismodified())
		zcat(result, "*");
	return result;
}

static char* get_control_info(char* result, void* p)
{
	result[0] = 0;
	return static_cast<draw::control*>(p)->getdescription(result);
}

static int layout_compare(const void* p1, const void* p2)
{
	layout* e1 = *((layout**)p1);
	layout* e2 = *((layout**)p2);
	if(e1->priority != e2->priority)
		return e2->priority - e1->priority;
	return strcmp(e2->name[0], e1->name[0]);
}

static void layout_initialize()
{
	layout** p1 = layouts;
	for(auto p = layout::first; p; p = p->next)
		*p1++ = p;
	*p1 = 0;
	qsort(layouts, p1 - layouts, sizeof(layouts[0]), layout_compare);
}

static void set_title(const char* p)
{
	if(!p)
		return;
	char temp[512];
	szprint(temp, p);
	setcaption(temp);
}

static int show_layout(rect rc)
{
	int height = texth() + 12;
	if(rc.height() > height)
	{

		rc.y1 = rc.y1 + (rc.height() - height) / 2;
		rc.y2 = rc.y1 + height;
	}
	rc.y1 += 2;
	rc.y2 -= 3;
	int x2 = rc.x2;
	for(auto p : layouts)
	{
		if(!p)
			break;
		unsigned state = 0;
		if(strcmp(p->name[0], layout_name) == 0)
			state |= Checked;
		const char* name = p->name[0];
		rc.x1 = rc.x2 - textw(name) - 4 * 2 + 1;
		if(areb(rc))
			statusbar("Переключиться на %1.", name);
		if(tool(rc, false, false, true))
			layout_param = p->name[0];
		text(rc, name, AlignCenterCenter);
		rc.x2 = rc.x1 - 1;
	}
	return x2 - rc.x2;
}

void layout::background(rect& rc, rect& rt, unsigned& flags)
{
	setclip();
	int tools_height = 0;
	if(metrics::toolbar)
	{
		if(toolheight)
			tools_height = toolheight;
		else
			tools_height = 24 + 6 * 2;
	}
	rc.set(0, 0, getwidth(), getheight());
	rt = rc;
	statusbar("Готово (для справки нажмите F1)");
	rc.y1 += sheetline({rc.x1, rc.y1, rc.x2, rc.y1 + tools_height}, 0);
	rt.y2 = rc.y1; rt.offset(3, 3);
	rectf(rc, colors::form);
	rc.y2 -= wdt_statusbar();
	if(metrics::show::padding)
		rc.offset(metrics::padding, metrics::padding + 1);
	rt.x2 -= show_layout(rt);
}

control* dialog::find(const char* url)
{
	if(url[0] == 0)
		return 0;
	char temp[260];
	for(auto e : objects)
	{
		auto p = e->geturl(temp);
		if(!p)
			continue;
		if(strcmp(p, url) == 0)
			return e;
	}
	return 0;
}

control* dialog::openbyurl(const char* url, bool make_active)
{
	auto p = find(url);
	if(!p)
	{
		p = draw::control::createbyurl(url);
		if(!p)
			return 0;
		objects.reserve();
		objects.add(p);
	}
	if(make_active)
		active_workspace_tab = p;
	return p;
}

void dialog::remove(draw::control* object)
{
	int index = objects.indexof(object);
	objects.remove(index);
	if(index != -1 && object == active_workspace_tab)
	{
//		if(index >= objects.count)
//			index--;
//		if(index < 0)
//			active_workspace_tab = 0;
//		else
//			active_workspace_tab = objects.data[index];
	}
//	draw::cleanup(object);
//	if(!object->isstatic())
//		delete object;
}

int dialog::select(draw::control** result)
{
	auto p = result;
	for(auto e : objects)
		*p++ = e;
	*p = 0;
	return p - result;
}

static int getindex(draw::control** p1, int count, const draw::control* e)
{
	for(int i = 0; i < count; i++)
	{
		if(p1[i] == e)
			return i;
	}
	return -1;
}

static void workspace(rect rc, unsigned flags, bool allow_multiply_window)
{
	control* p1[64];
	int c1 = getdocked(p1, sizeof(p1)/ sizeof(p1[0]), DockWorkspace);
	int c2 = c1 + dialog::select(p1 + c1);
	if(c1 == 1 && !allow_multiply_window)
	{
		active_workspace_tab = p1[0];
		view(rc, active_workspace_tab[0], false);
	}
	else if(c1 || c2)
	{
		auto last_active_workspace_tab = active_workspace_tab;
		int current_select = getindex(p1, c2, active_workspace_tab);
		if(current_select == -1)
			current_select = 0;
		active_workspace_tab = p1[current_select];
		auto ec = active_workspace_tab;
		const int dy = draw::texth() + 8;
		rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
//		rct.x1 += draw::tabs(rct, TabsControl, HideClose | HideBackground, (void**)p1, 0, c1,
//			current_select, &current_select, get_control_name, 0, 2);
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
		view(rc, *ec, false);
	}
}

control* dialog::getworking()
{
	return active_workspace_tab;
}

static struct layout_application : layout
{
	int redraw() override
	{
		rect rc, rt;
		unsigned flags;
		while(true)
		{
			background(rc, rt, flags);
			dockbar(rc);
			workspace(rc, flags, allow_multiply_window);
			if(metrics::toolbar)
			{
				//for(auto p = draw::dialog::commandset::first; p; p = p->next)
				//	rt.x1 += toolbar(rt, 1, HideBackground, p->source);
				//auto pd = dialog::getworking();
				//if(pd)
				//	rt.x1 += toolbar(rt, 1, HideBackground, pd->getcommands(), pd);
			}
			int id = input();
			//auto active = getactive();
			//switch(id)
			//{
			//case 0:
			//	return 0;
			//case Shift | F2:
			//	metrics::show::left = !metrics::show::left;
			//	break;
			//case F2:
			//	metrics::show::bottom = !metrics::show::bottom;
			//	break;
			//case Ctrl | F2:
			//	metrics::show::right = !metrics::show::right;
			//	break;
			//case TabsCloseControl:
			//	if(close_workspace_tab)
			//		dialog::remove(close_workspace_tab);
			//	break;
			//default:
			//	if(ischanging())
			//		return Commands;
			//	if(definput())
			//		break;
			//	// Обработаем стандартные команды
			//	for(auto p = draw::dialog::commandset::first; p; p = p->next)
			//	{
			//		if(p->execute(id, true, active) == 0)
			//			continue;
			//		break;
			//	}
			//	break;
			//}
		}
	}

	layout_application()
	{
		name[0] = "Main";
		name[1] = "Главный";
		layout_name = name[0];
	}

} layout_application_instance;

//dialog::commandset* dialog::commandset::first;
//
//dialog::commandset::commandset()
//{
//	seqlink(this);
//}

//static struct standart_command : public dialog::commandset
//{
//
//	char* add_filter(char* result, const char* name, const char* extension)
//	{
//		if(!extension)
//			return result;
//		zcat(result, name);
//		szupper(result, 1);
//		result += zlen(result); *result++ = 0; *result = 0;
//		szprint(result, "*.%1", extension);
//		return result;
//	}
//
//	char* add_filter(char* result, const char* name, const char** extensions)
//	{
//		if(!extensions)
//			return result;
//		zcat(result, name);
//		szupper(result, 1);
//		result += zlen(result); *result++ = 0; *result = 0;
//		for(auto pe = extensions; *pe; pe++)
//		{
//			if(*result)
//				zcat(result, ";");
//			szprint(zend(result), "*.%1", *pe);
//		}
//		result += zlen(result); *result++ = 0; *result = 0;
//		return result;
//	}
//
//	void make_filter(char* result)
//	{
//		static const char* all_files[] = {"*", 0};
//		result[0] = 0;
//		result = add_filter(result, szt("All files", "Все файлы"), all_files);
//		for(auto p = draw::control::plugin::first; p; p = p->next)
//			result = add_filter(result, p->getdescription(), p->getextensions());
//	}
//
//	int execute(int id, bool run, control* element) override
//	{
//		switch(id)
//		{
//		case Create:
//			break;
//		case Open:
//			if(run)
//			{
//				char temp[260]; memset(temp, 0, sizeof(temp));
//				char filter[4096]; make_filter(filter);
//				if(dialog::open(0, temp, filter))
//					dialog::openbyurl(temp);
//			}
//			break;
//		case Save:
//			if(!element)
//				return Disabled;
//			if(run)
//			{
//				auto url = element->gets(Path);
//				if(!url || url[0] == 0)
//				{
//					char temp[260]; temp[0] = 0;
//					char filter[260]; filter[0] = 0;
//					auto pname = element->gets(Identifier);
//					if(!pname || pname[0] == 0)
//						pname = "All files";
//					auto pext = element->gets(element->findparam("extension"));
//					if(!pext || pext[0] == 0)
//						pext = "*";
//					add_filter(filter, pname, pext);
//					if(dialog::save(0, temp, filter))
//						element->sets(Path, temp);
//				}
//			}
//			break;
//		default:
//			return 0;
//		}
//		return Executed;
//	}
//
//	standart_command() : commandset()
//	{
//		static menu source[] = {
//			{Create},
//			{Open},
//			{Save},
//			{-1},
//			{0}
//		};
//		this->source = source;
//	}
//
//} standart_command_instance;

static bool button(int x, int y, int width, int height, int id, const char* string, unsigned state, int key, bool press, color c1)
{
	color c2 = (c1.gray().r > 128) ? c1.lighten() : c1.darken();
	bool result = false;
	struct rect rc = {x, y, x + width, y + height};
	switch(area(rc))
	{
	case AreaHilited:
		gradv(rc, c2.lighten(), c2.darken());
		if(hot::key == MouseLeft && hot::pressed == press)
			result = true;
		break;
	case AreaHilitedPressed:
		gradv(rc, c2.darken(), c2.lighten());
		if(hot::key == MouseLeft)
		{
			if(hot::pressed == press)
				result = true;
			else if((state&Disabled) == 0)
			{
				//setfocus(id);
				state |= Focused;
			}
		}
		break;
	default:
		gradv(rc, c1.lighten(), c1.darken());
		break;
	}
	if(state&Disabled)
		rectb(rc, colors::border.mix(colors::window));
	else
	{
		if((state&Focused) != 0 && key && hot::key == key)
			result = true;
		rectb(rc, (state&Focused) ? colors::active : colors::border);
	}
	if(string)
	{
		draw::state push;
		if(c1.gray().r > 160)
			fore = colors::black;
		else
			fore = colors::white;
		text(rc, string, AlignCenterCenter);
	}
	return result;
}

static int button(int x, int y, int width, int id, unsigned state, color c1)
{
	char temp[64];
	int height = 2 * 4 + texth();
	rect rc = {x, y, x + width, y + height};
	//focusing(rc, id, state);
	if(button(x, y, width, height, id, c1.getname(temp), state, 0, false, c1))
		execute(id);
	return height;
}

static const char* gtitle(char* temp, settings& e)
{
	zcpy(temp, e.name);
	szupper(temp);
	zcat(temp, ":");
	return temp;
}

static const char* gtext(char* temp, settings& e)
{
	zcpy(temp, e.name);
	szupper(temp);
	return temp;
}

static int compare_settings(const void* p1, const void* p2)
{
	const settings* e1 = *((settings**)p1);
	const settings* e2 = *((settings**)p2);
	if(e1->priority != e2->priority)
		return e1->priority - e2->priority;
	return strcmp(e1->name, e2->name);
}

static void get_tabs_child(settings** result, settings* parent)
{
	settings** ps = result;
	settings* tabs = parent->child();
	if(tabs)
	{
		for(settings* p = tabs; p; p = p->next)
		{
			if(p->e_visible && !p->e_visible(*p))
				continue;
			*ps++ = p;
		}
	}
	*ps = 0;
	qsort(result, zlen(result), sizeof(result[0]), compare_settings);
}

static struct settings_header_list_class : controls::list
{
	settings* rows[128];

	void initialize()
	{
		get_tabs_child(rows, &settings::root);
		maximum = zlen(rows);
		if(current >= maximum - 1)
			current = maximum;
		if(current < 0)
			current = 0;
	}

	void row(rect rc, int index) override
	{
		if(index == current)
			hilight({rc.x1, rc.y1, rc.x2 - 1, rc.y2}, false);
		textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 8, rows[index]->name);
	}

	settings* gcurrent()
	{
		return rows[current];
	}

} header;

static int title(int& x, int y, int width, unsigned state, const char* string)
{
	int w = 200;
	rect rc = {x, y, x + w, y};
	textw(rc, string); text({rc.x1, rc.y1 + 2, rc.x2, rc.y2 + 2}, string, AlignLeft);
	x += w;
	return rc.height() + 4;
}

static int count(settings* e)
{
	if(!e)
		return 0;
	if(e->e_visible && !e->e_visible(*e))
		return 0;
	int result = 0;
	switch(e->type)
	{
	case settings::Group:
		for(settings* t = e->child(); t; t = t->next)
			result += count(t);
		return result;
	default:
		return 1;
	}
}

static int selement(int x, int y, int width, int& id, unsigned state, settings& e)
{
	int cnt;
	int w;
	int x2 = x + width;
	settings* pc;
	char temp[512];
	if(e.e_visible && !e.e_visible(e))
		return 0;
	int y1 = y;
	int h1;
	elements[id - FirstControl] = &e;
	temp[0] = 0;
	draw::element we;
	we.label = temp;
	switch(e.type)
	{
	case settings::Radio:
		y += radio(x, y, width, id, state | ((*((int*)e.data) == e.value) ? Checked : 0), gtext(temp, e));
		we.type = wdt_radio;
		if(*((int*)e.data) == e.value)
			we.flags = state | Checked;
		break;
	case settings::Bool:
		we.type = wdt_check;
		y += checkbox(x, y, width, id, state | (*((bool*)e.data) ? Checked : 0), gtext(temp, e));
		break;
	case settings::Int:
		if(e.value)
			cnt = (getdigitscount(e.value) + 1)*textw("0") + metrics::padding * 2 + 19;
		else
			cnt = x2 - x;
		if(cnt > x2 - x)
			cnt = x2 - x;
		h1 = title(x, y, width, state, gtitle(temp, e));
		y += imax(edit(x, y, cnt, id, state, *((int*)e.data)), h1);
		break;
	case settings::Color:
		h1 = title(x, y, width, state, gtitle(temp, e));
		y += imax(button(x, y, x2 - x, id, state, *((color*)e.data)), h1);
		break;
	case settings::Button:
		y += button(x, y, x2 - x, id, state, gtext(temp, e));
		break;
	case settings::UrlFolder:
		h1 = title(x, y, width, state, gtitle(temp, e));
		w = texth() + 8;
		// Установим статус фокуса,
		// чтобы работали горячие клавиши
		if(getfocus() == id)
			state |= Focused;
		// Кнопка закрытия
		if(draw::buttonh(x2 - w, y, w, w, state, "...", F3, false, ClipartNone, "Выберите папку"))
		{
			setfocus(id);
			execute(InputChoose);
		}
		x2 -= w;
		y += imax(edit(x, y, x2 - x, id, state, ((char*)e.data)), h1);
		break;
	case settings::Group:
		pc = e.child();
		if(!pc)
			return 0;
		id++;
		x += metrics::padding;
		width -= metrics::padding * 4;
		y += getgroupheight() + metrics::padding * 2;
		for(; pc; pc = pc->next)
		{
			int h = selement(x, y, width, id, state, *pc);
			if(h)
				y += h + metrics::padding;
		}
		group({x - metrics::padding * 2, y1, x + width + metrics::padding * 4, y + metrics::padding}, e.name[locale]);
		y += metrics::padding * 2;
		return y - y1;
	default:
		break;
	}
	id++;
	return y - y1;
}

static char* tab_text(char* temp, void* p)
{
	return (char*)((settings*)p)->name[locale];
}

static struct layout_settings : layout
{
	int redraw() override
	{
		rect rc, rt;
		unsigned flags;
		char temp[512];
		::settings* tabs[128];
		header.initialize();
		int cash_locale = locale;
		while(true)
		{
			fore = colors::text;
			font = metrics::font;
			background(rc, rt, flags);
			unsigned state = 0;
			splitv(rc.x1, rc.y1, header_width, rc.height(), 1, 6, 64, 282);
			view({rc.x1, rc.y1, rc.x1 + header_width, rc.y2}, 1,
				HideActiveBorder | (metrics::show::padding ? 0 : HideBorder),
				header);
			rc.x1 += header_width + 6;
			::settings* top = header.gcurrent();
			// При изменении текущего заголовка
			if(top != current_header)
			{
				current_header = top;
				current_tab = -1;
			}
			if(top)
			{
				get_tabs_child(tabs, top);
				if(tabs[0])
				{
					// Покажем дополнительную панель
					if(current_tab == -1)
						current_tab = 0;
					int h1 = 28;
					// Нарисуем закладки
					draw::tabs({rc.x1, rc.y1, rc.x2, rc.y1 + h1 + 1}, Tabs, HideClose,
						(void**)tabs, 0, zlen(tabs), current_tab, 0,
						tab_text, 0, metrics::padding, 0, metrics::padding);
					if(metrics::show::padding)
						rectb(rc, colors::border);
					line(rc.x1, rc.y1 + h1, rc.x2, rc.y1 + h1, colors::border);
					rc.y1 += h1 + metrics::padding * 3;
					rc.x1 += metrics::padding * 3;
					rc.x2 -= metrics::padding * 3;
					// Нариуем текущую закладку
					if(current_tab != -1)
					{
						int nc = FirstControl;
						int w4 = rc.width();
						int w3 = imin(w4, 640);
						::settings* p1 = tabs[current_tab];
						switch(p1->type)
						{
						case settings::Control:
							view({rc.x1 - metrics::padding, rc.y1,
								rc.x2 + metrics::padding, rc.y2 - metrics::padding}, nc, state, *((control*)p1->data));
							break;
						default:
							for(::settings* p = p1->child(); p; p = p->next)
							{
								int h = selement(rc.x1, rc.y1, w3, nc, state, *p);
								if(h)
									rc.y1 += h + metrics::padding;
							}
							break;
						}
					}
				}
			}
			int id = input();
			switch(id)
			{
			case 0:
			case KeyEscape:
				return 0;
			case InputChoose:
				id = getfocus();
				if(id >= FirstControl && id <= LastControl)
				{
					::settings& e = *elements[id - FirstControl];
					bool result = true;
					switch(e.type)
					{
					case settings::UrlFolder:
						result = dialog::folder("Выберите папку", (char*)e.data);
						break;
					default:
						break;
					}
					if(result && e.e_execute)
						e.e_execute();
				}
				break;
			case InputEdit:
				if(hot::param >= FirstControl && hot::param <= LastControl)
				{
					id = hot::param;
					::settings& e = *elements[id - FirstControl];
					bool result = true;
					switch(e.type)
					{
					case settings::Text:
						zcpy(temp, (char*)e.data, sizeof(temp));
						if(editing(hot::element, metrics::edit, temp, 0, imin(e.value, (int)sizeof(temp) - 1), false))
							zcpy((char*)e.data, temp, e.value);
						break;
					case settings::UrlFolder:
						zcpy(temp, (char*)e.data, sizeof(temp));
						if(editing(hot::element, metrics::edit, temp, 0, imin(e.value, (int)sizeof(temp) - 1), false))
							zcpy((char*)e.data, szurlc(temp), e.value);
						break;
					case settings::TextPtr:
						zcpy(temp, *((char**)e.data), sizeof(temp) - 1);
						if(editing(hot::element, metrics::edit, temp, 0, imin(e.value, (int)sizeof(temp) - 1), false))
							*((const char**)e.data) = szdup(temp);
						break;
					case settings::UrlFolderPtr:
						zcpy(temp, *((char**)e.data), sizeof(temp) - 1);
						if(editing(hot::element, metrics::edit, temp, 0, imin(e.value, (int)sizeof(temp) - 1), false))
							*((const char**)e.data) = szdup(szurlc(temp));
						break;
					case settings::Int:
						sznum(temp, *((int*)e.data));
						if(editing(hot::element, metrics::edit, temp, AlignRight, sizeof(temp) - 1, false))
						{
							*((int*)e.data) = sz2num(temp);
							if(*((int*)e.data) > e.value)
								*((int*)e.data) = e.value;
						}
						break;
					default:
						break;
					}
					if(result && e.e_execute)
						e.e_execute();
				}
				break;
			case InputEditPlus:
				if(hot::param >= FirstControl && hot::param <= LastControl)
				{
					id = hot::param;
					::settings& e = *elements[id - FirstControl];
					bool result = true;
					switch(e.type)
					{
					case settings::Int:
						*((int*)e.data) = *((int*)e.data) + 1;
						if(*((int*)e.data) > e.value)
							*((int*)e.data) = e.value;
						break;
					default:
						break;
					}
					if(result && e.e_execute)
						e.e_execute();
				}
				break;
			case InputEditMinus:
				if(hot::param >= FirstControl && hot::param <= LastControl)
				{
					id = hot::param;
					::settings& e = *elements[id - FirstControl];
					bool result = true;
					switch(e.type)
					{
					case settings::Int:
						*((int*)e.data) = *((int*)e.data) - 1;
						if(*((int*)e.data) < 0)
							*((int*)e.data) = 0;
						break;
					default:
						break;
					}
					if(result && e.e_execute)
						e.e_execute();
				}
				break;
			case Tabs:
				current_tab = hot::param;
				break;
			default:
				if(ischanging())
					return Commands;
				if(id >= FirstControl && id <= LastControl)
				{
					::settings& e = *elements[id - FirstControl];
					bool result = true;
					switch(e.type)
					{
					case settings::Bool:
						*((bool*)e.data) = !*((bool*)e.data);
						break;
					case settings::Radio:
						*((int*)e.data) = e.value;
						break;
					case settings::Color:
						result = dialog::color(*((::color*)e.data));
						break;
					default:
						break;
					}
					if(result && e.e_execute)
						e.e_execute();
				}
				else
					definput();
				break;
			}
		}
	}

	layout_settings()
	{
		name[0] = "Settings";
		name[1] = "Настройки";
	}

} layout_settings_instance;

void set_light_theme();
void set_dark_theme();

static const char* get_lang_name(const char* url)
{
	char temp[260];
	return szdup(szfnamewe(temp, url));
}

static void setting_appearance_general_metrics()
{
	settings& e1 = settings::root.gr("Рабочий стол").gr("Общие").gr("Метрика");
	e1.add("Отступы", metrics::padding);
	e1.add("Ширина скролла", metrics::scroll);
}

static void setting_appearance_forms()
{
	settings& e2 = settings::root.gr("Цвета").gr("Общие");
	e2.add("Установить светлую тему", set_light_theme);
	e2.add("Установить темную тему", set_dark_theme);
	settings& e3 = settings::root.gr("Цвета").gr("Формы");
	e3.add("Цвет текста", colors::text);
	e3.add("Цвет окна", colors::window);
	e3.add("Цвет формы", colors::form);
	e3.add("Цвет границы", colors::border);
	e3.add("Активный цвет", colors::active);
	e3.add("Цвет кнопки", colors::button);
	e3.add("Цвет редактирования", colors::edit);
	e3.add("Цвет закладок", colors::tabs::back);
	e3.add("Цвет текста закладок", colors::tabs::text);
	e3.add("Цвет подсказки", colors::tips::back);
	e3.add("Цвет текста подсказки", colors::tips::text);
}

static void setting_appearance_general_view()
{
	settings& e1 = settings::root.gr("Рабочий стол").gr("Общие").gr("Вид");
	e1.add("Показывать панель статуса", metrics::show::statusbar);
	e1.add("Показывать левую панель элементов", metrics::show::left);
	e1.add("Показывать правую панель элементов", metrics::show::right);
	e1.add("Показывать нижнюю панель элементов", metrics::show::bottom);
	e1.add("Отступы на главном окне", metrics::show::padding);
	e1.add("Использовать оптимизацию при движении мишки", sys_optimize_mouse_move);
}

//#if _DEBUG
//COMMAND(after_render)
//{
//	draw::rectf(sys_static_area, colors::white, 96);
//}
//#endif

static void setting_workspace_general_tabs()
{
	settings& e1 = settings::root.gr("Рабочий стол").gr("Закладки").gr("Имена");
	e1.add("Показывать расширения файлов", show_file_extension);
	e1.add("Первая буква в верхнем регистре", first_letter_to_uppercase);
}

//static menu docking_values[] = {
//	{DockLeft, "left_panel", {"Left panel", "Левая панель"}},
//	{DockLeftBottom, "left_panel_bottom", {"Left panel (bottom)", "Левая панель (низ)"}},
//	{DockRight, "right_panel", {"Right panel", "Правая панель"}},
//	{DockRightBottom, "right_panel_bottom", {"Right panel (bottom)", "Правая панель (низ)"}},
//	{DockBottom, "bottom_panel", {"Bottom panel", "Нижняя панель"}},
//	{DockWorkspace, "workspace", {"Workspace", "Рабочий стол"}},
//	{0}
//};

//static void setting_plugins_controls()
//{
//	struct table_class : controls::tree
//	{
//
//		bool changing(void* object, menu& e) override
//		{
//			if(indexof(object) != -1)
//				object = (void*)((element*)object)->param;
//			if(e.id == Docking && !static_cast<control::plugin*>(object)->instance)
//				return false;
//			return tree::changing(object, e);
//		}
//
//		virtual bool get(const void* object, const menu& e, char* text) const override
//		{
//			const char* p;
//			if(indexof(object) != -1)
//				object = (void*)((element*)object)->param;
//			switch(e.id)
//			{
//			case Name:
//				p = static_cast<const control::plugin*>(object)->getdescription();
//				if(p)
//					zcpy(text, p, 128);
//				break;
//			case Docking:
//				if(!static_cast<const control::plugin*>(object)->instance)
//				{
//					zcpy(text, szt("Not available", "Не доступно"));
//					return true;
//				}
//				return table::get(object, e, text);
//			default:
//				return table::get(object, e, text);
//			}
//			return true;
//		}
//
//		virtual int get(const void* object, const menu& e) const override
//		{
//			if(indexof(object) != -1)
//				object = (void*)((element*)object)->param;
//			switch(e.id)
//			{
//			case Docking:
//				return static_cast<const control::plugin*>(object)->type;
//			case Visibility:
//				return static_cast<const control::plugin*>(object)->disabled ? 0 : 1;
//			default:
//				return 0;
//			}
//		}
//
//		virtual void set(void* object, const menu& e, int value) override
//		{
//			if(indexof(object) != -1)
//				object = (void*)((element*)object)->param;
//			switch(e.id)
//			{
//			case Docking:
//				((control::plugin*)object)->type = value;
//				break;
//			case Visibility:
//				((control::plugin*)object)->disabled = (value == 0);
//				break;
//			}
//		}
//
//	};
//	static table_class table;
//	table.addcol(Visibility, Boolean, 0, -1, ColumnSizeAuto);
//	table.addcol(Name, Text, 0, -1, ColumnSizeAuto | ColumnReadOnly);
//	table.addcol(Docking, Number, 0, 200, ColumnSizeNormal, docking_values);
//	table.validate();
//	for(auto p = control::plugin::first; p; p = p->next)
//	{
//		if(!p)
//			break;
//		table.addrow(p);
//	}
//	if(!table.count)
//		return;
//	table.no_change_count = true;
//	table.commands = 0;
//	settings& e1 = settings::root.gr("Plugins", "Модули");
//	e1.add("Controls", "Элементы", table);
//}

void dialog::initialize()
{
	set_light_theme();
	setting_appearance_general_view();
	setting_workspace_general_tabs();
	setting_appearance_general_metrics();
	setting_appearance_forms();
	setting_localization();
	//setting_plugins_controls();
	layout_initialize();
	command_app_initialize->execute();
}

int dialog::application(const char* title, unsigned window_flags, bool multiply_window)
{
	if(window_flags == -1)
		window_flags = WFMinmax | WFResize;
	window dc(-1, -1, 640, 480, window_flags, 32, "ApplicationWindow");
	if(title)
		set_title(title);
	allow_multiply_window = multiply_window;
	while(true)
	{
		bool painted = false;
		for(auto e : layouts)
		{
			if(!e)
				break;
			if(strcmp(e->name[0], layout_name) != 0)
				continue;
			if(!e->redraw())
				return 0;
			painted = true;
			setfocus(0);
			if(!layout_name)
				return 0;
		}
		if(!painted)
			return 0;
	}
}