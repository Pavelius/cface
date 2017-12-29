#include "crt.h"
#include "command.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

static rect	statusbar_rect;
static char	statusbar_text[512];

void statusbar(const char* format, ...)
{
	szprintv(statusbar_text, format, xva_start(format));
}

int draw::statusbardraw()
{
	if(!metrics::show::statusbar || !font)
		return 0;
	int dx = font->height + 6;
	statusbar_rect.set(0, getheight() - dx, getwidth(), getheight());
	gradv(statusbar_rect, colors::button.lighten(), colors::button.darken());
	line(statusbar_rect.x1, statusbar_rect.y1,
		statusbar_rect.x2, statusbar_rect.y1, colors::border);
	statusbar_rect.offset(4, 3);
	return dx;
}

COMMAND(after_render)
{
	// ¬ задачи этого расширени€ вывести короткое сообщение статуса
	// в специальную область под названием 'область статуса'.
	// “екст сообщени€ можт определ€тьс€ в процессе рендеринга
	// и он должен наложитьс€ на экран в указанную область
	// если нет больше никаких других команд
	if(!statusbar_rect || !metrics::show::statusbar)
		return;
	if(statusbar_text[0])
		draw::text(statusbar_rect, statusbar_text);
	statusbar_rect.clear();
	statusbar_text[0] = 0;
}