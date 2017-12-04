#include "command.h"
#include "crt.h"
#include "draw.h"
#include "draw_table.h"
#include "settings.h"

using namespace	draw;
using namespace	draw::controls;

bool				metrics::show::padding;
static int			current_tab;
static settings*	current_header;

static char* gettabname(char* temp, void* p)
{
	return (char*)((settings*)p)->name;
}

static void callback_settab()
{
	current_tab = hot::param;
}

static int compare_settings(const void* p1, const void* p2)
{
	const settings* e1 = *((settings**)p1);
	const settings* e2 = *((settings**)p2);
	if(e1->priority != e2->priority)
		return e1->priority - e2->priority;
	return strcmp(e1->name, e2->name);
}

static void getsiblings(settings** result, unsigned maximum_count, settings* parent)
{
	settings** ps = result;
	settings** pe = result + maximum_count - 1;
	settings* tabs = parent->child();
	if(tabs)
	{
		for(settings* p = tabs; p; p = p->next)
		{
			if(p->e_visible && !p->e_visible(*p))
				continue;
			if(ps < pe)
				*ps++ = p;
		}
	}
	*ps = 0;
	qsort(result, zlen(result), sizeof(result[0]), compare_settings);
}

static void showcontrol(control& e, rect rc)
{
	unsigned flags = 0;
	focusing((int)&e, flags, rc);
	e.focused = (flags & Focused) != 0;
	e.view(rc, false);
}

static void callback_button()
{
	auto p = (settings*)hot::param;
	if(p->e_execute)
		p->e_execute();
}

static void callback_bool()
{
	auto p = (settings*)hot::param;
	*((bool*)p->data) = !*((bool*)p->data);
}

static void callback_radio()
{
	auto p = (settings*)hot::param;
	*((int*)p->data) = p->value;
}

static void callback_up()
{
	auto p = (settings*)hot::param;
	(*((int*)p->data))--;
}

static void callback_down()
{
	auto p = (settings*)hot::param;
	(*((int*)p->data))++;
}

static void callback_choose_folder()
{
	char temp[260]; temp[0] = 0;
	auto p = (settings*)hot::param;
	auto v = *((const char**)p->data);
	if(v)
		zcpy(temp, v);
	if(draw::dialog::folder("Укажите папку", temp))
	{
		if(temp[0])
			*((const char**)p->data) = szdup(temp);
		else
			*((const char**)p->data) = 0;
	}
}

static void callback_choose_color()
{
	auto p = (settings*)hot::param;
	draw::dialog::color(*((color*)p->data));
}

static struct widget_settings_header : list
{
	settings* rows[128];

	void initialize()
	{
		getsiblings(rows, sizeof(rows) / sizeof(rows[0]), &settings::root);
		maximum = zlen(rows);
		if(current >= maximum - 1)
			current = maximum;
		if(current < 0)
			current = 0;
	}

	void row(rect rc, int index) override
	{
		list::row({rc.x1 + 1, rc.y1 + 1, rc.x2 - 1, rc.y2}, index);
		textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 8, rows[index]->name);
	}

	settings* getcurrent()
	{
		return rows[current];
	}

} header;

static struct widget_settings : control
{

	int header_width;

	static const char* getname(char* temp, settings& e)
	{
		zcpy(temp, e.name);
		szupper(temp);
		return temp;
	}

	static int buttonc(int x, int y, int width, int id, unsigned flags, color value, const char* tips, void(*callback)())
	{
		char temp[128]; szprint(temp, "%1i, %2i, %3i", value.r, value.g, value.b);
		setposition(x, y, width);
		struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
		focusing(id, flags, rc);
		if(buttonh({x, y, x + width, rc.y2},
			ischecked(flags), isfocused(flags), isdisabled(flags), true, value,
			temp, KeyEnter, false, tips))
		{
			draw::execute(callback);
			hot::param = id;
		}
		return rc.height() + metrics::padding * 2;
	}

	static int element(int x, int y, int width, unsigned flags, settings& e)
	{
		const auto title = 160;
		settings* pc;
		char temp[512]; temp[0] = 0;
		if(e.e_visible && !e.e_visible(e))
			return 0;
		int y1 = y;
		switch(e.type)
		{
		case settings::Radio:
			y += radio(x, y, width, (int)&e, flags | ((*((int*)e.data) == e.value) ? Checked : 0),
				getname(temp, e), 0, callback_radio);
			break;
		case settings::Bool:
			y += checkbox(x, y, width, (int)&e, flags | (*((bool*)e.data) ? Checked : 0),
				getname(temp, e), 0, callback_bool);
			break;
		case settings::Int:
			if(e.value) // Есть максимум
			{
				auto w = (getdigitscount(e.value) + 1)*textw("0") + metrics::padding * 2 + 19;
				if(title + w < width)
					width = title + w;
			}
			sznum(temp, *((int*)e.data));
			titletext(x, y, width, flags, e.name, title);
			y += field(x, y, width, (int)&e, flags, temp, 0, 0, 0, 0, callback_up, callback_down);
			break;
		case settings::Color:
			titletext(x, y, width, flags, e.name, title);
			y += buttonc(x, y, width, (int)&e, flags, *((color*)e.data), 0, callback_choose_color);
			break;
		case settings::Button:
			y += button(x, y, width, (int)&e, flags, getname(temp, e), 0, callback_button);
			break;
		case settings::TextPtr:
			titletext(x, y, width, flags, e.name, title);
			y += field(x, y, width, (int)&e, flags, *((const char**)e.data));
			break;
		case settings::UrlFolderPtr:
			titletext(x, y, width, flags, e.name, title);
			y += field(x, y, width, (int)&e, flags, *((const char**)e.data), 0, 0, 0, callback_choose_folder);
			break;
		case settings::Control:
			break;
		case settings::Group:
			pc = e.child();
			if(!pc)
				return 0;
			if(true)
			{
				setposition(x, y, width); auto y2 = y;
				auto height = draw::texth() + metrics::padding * 2;
				y += height;
				for(; pc; pc = pc->next)
					y += element(x, y, width, flags, *pc);
				if(e.name)
				{
					color c1 = colors::border.mix(colors::window, 128);
					color c2 = c1.darken();
					gradv({x, y2, x + width, y2 + height}, c1, c2);
					fore = colors::text.mix(c1, 96);
					text(x + (width - textw(e.name)) / 2, y2 + metrics::padding, e.name);
					rectb({x, y2, x + width, y + metrics::padding}, colors::border);
				}
				y += metrics::padding;
			}
			break;
		}
		return y - y1;
	}

	void redraw(rect rc)
	{
		draw::state push;
		settings* tabs[128];
		fore = colors::text;
		splitv(rc.x1, rc.y1, header_width, rc.height(), 1, 6, 64, 282);
		showcontrol(header, {rc.x1, rc.y1, rc.x1 + header_width, rc.y2});
		rc.x1 += header_width + 6;
		auto top = header.getcurrent();
		// При изменении текущего заголовка
		if(top != current_header)
		{
			current_header = top;
			current_tab = -1;
		}
		if(!top)
			return;
		getsiblings(tabs, sizeof(tabs) / sizeof(tabs[0]), top);
		if(tabs[0])
		{
			// Покажем дополнительную панель
			if(current_tab == -1)
				current_tab = 0;
			int h1 = 28;
			// Нарисуем закладки
			auto hilited = -1;
			if(draw::tabs({rc.x1, rc.y1, rc.x2, rc.y1 + h1 + 1}, false, false,
				(void**)tabs, 0, zlen(tabs), current_tab, &hilited,
				gettabname))
			{
				draw::execute(callback_settab);
				hot::param = hilited;
			}
			if(metrics::show::padding)
				rectb(rc, colors::border);
			line(rc.x1, rc.y1 + h1, rc.x2, rc.y1 + h1, colors::border);
			rc.y1 += h1 + metrics::padding * 2;
			rc.x1 += metrics::padding;
			rc.x2 -= metrics::padding;
			// Нариуем текущую закладку
			if(current_tab != -1)
			{
				int w4 = rc.width();
				int w3 = imin(w4, 640);
				auto p1 = tabs[current_tab];
				switch(p1->type)
				{
				case settings::Control:
					((control*)p1->data)->view(rc, true);
					break;
				default:
					for(auto p = p1->child(); p; p = p->next)
						rc.y1 += element(rc.x1, rc.y1, w3, 0, *p);
					break;
				}
			}
		}
	}

	widget_settings() : header_width(160)
	{
		show_background = false;
		show_border = false;
	}

} widget_settings_control;

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
	//e1.add("Использовать оптимизацию при движении мишки", sys_optimize_mouse_move);
}

static void initialize_settings()
{
	setting_appearance_general_metrics();
	setting_appearance_forms();
	setting_appearance_general_view();
	header.initialize();
}

int draw::application(const char* title)
{
	initialize_settings();
	draw::window dc(-1, -1, 600, 400, WFResize | WFMinmax);
	if(title)
		draw::setcaption(title);
	while(true)
	{
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form); rc.offset(metrics::padding);
		widget_settings_control.view(rc, true);
		auto id = draw::input();
		if(!id)
			return 0;
		control::dodialog(id);
	}
}