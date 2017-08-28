#include "crt.h"
#include "command.h"
#include "draw.h"

rect	tooltips_rect;
char	tooltips_text[4096];

void tooltips(rect rc, const char* format, ...)
{
	if(hot::key != InputIdle)
		return;
	tooltips_rect = rc;
	szprintv(tooltips_text, format, xva_start(format));
}

void tooltips(int x1, int y1, const char* format, ...)
{
	if(hot::key != InputIdle)
		return;
	tooltips_rect = {x1, y1, x1 + 400, y1};
	szprintv(tooltips_text, format, xva_start(format));
}

void tooltips(const char* format, ...)
{
	if(hot::key != InputIdle)
		return;
	int x1 = hot::mouse.x;
	int y1 = hot::mouse.y + 24;
	tooltips_rect = {x1, y1, x1 + 400, y1};
	szprintv(tooltips_text, format, xva_start(format));
}

COMMAND(after_render)
{
	if(!tooltips_text[0] || !tooltips_rect)
		return;
	draw::state push;
	draw::font = metrics::font;
	if(draw::font)
	{
		rect rc = tooltips_rect;
		if(tooltips_rect.y1 == tooltips_rect.y2)
		{
			draw::textf(rc, tooltips_text);
			rc.offset(-metrics::padding - 1, -metrics::padding);
		}
		// Correct border
		int height = draw::getheight();
		int width = draw::getwidth();
		if(rc.y2 >= height)
			rc.move(0, height - 2 - rc.y2);
		if(rc.x2 >= width)
			rc.move(width - 2 - rc.x2, 0);
		// Show background
		draw::rectf(rc, colors::tips::back);
		draw::rectb(rc, colors::border);
		rc.offset(metrics::padding);
		// Show text
		draw::fore = colors::tips::text;
		draw::textf(rc.x1, rc.y1, rc.width(), tooltips_text);
	}
	tooltips_rect.clear();
	tooltips_text[0] = 0;
}