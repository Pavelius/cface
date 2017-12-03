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

	static int element(int x, int y, int width, unsigned flags, settings& e)
	{
		int cnt;
		int x2 = x + width;
		settings* pc;
		char temp[512];
		if(e.e_visible && !e.e_visible(e))
			return 0;
		int y1 = y;
		temp[0] = 0;
		switch(e.type)
		{
		case settings::Radio:
			y += radio(x, y, width, (int)&e, flags | ((*((int*)e.data) == e.value) ? Checked : 0),
				getname(temp, e));
			break;
		case settings::Bool:
			y += checkbox(x, y, width, (int)&e, flags | (*((bool*)e.data) ? Checked : 0),
				getname(temp, e));
			break;
		case settings::Int:
			if(e.value)
				cnt = (getdigitscount(e.value) + 1)*textw("0") + metrics::padding * 2 + 19;
			else
				cnt = x2 - x;
			if(cnt > x2 - x)
				cnt = x2 - x;
			break;
		case settings::Color:
			break;
		case settings::Button:
			y += button(x, y, x2 - x, (int)&e, flags, getname(temp, e));
			break;
		case settings::UrlFolder:
			break;
		case settings::Group:
			pc = e.child();
			if(!pc)
				return 0;
			x += metrics::padding;
			width -= metrics::padding * 4;
			for(; pc; pc = pc->next)
			{
				int h = element(x, y, width, flags, *pc);
				if(h)
					y += h + metrics::padding;
			}
			y += metrics::padding * 2;
			return y - y1;
		}
		return y - y1;
	}

	void redraw(rect rc)
	{
		settings* tabs[128];
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
			rc.y1 += h1 + metrics::padding * 3;
			rc.x1 += metrics::padding * 3;
			rc.x2 -= metrics::padding * 3;
			// Нариуем текущую закладку
			if(current_tab != -1)
			{
				int w4 = rc.width();
				int w3 = imin(w4, 640);
				auto p1 = tabs[current_tab];
				switch(p1->type)
				{
				case settings::Control:
					break;
				default:
					for(auto p = p1->child(); p; p = p->next)
					{
						int h = element(rc.x1, rc.y1, w3, 0, *p);
						if(h)
							rc.y1 += h + metrics::padding;
					}
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