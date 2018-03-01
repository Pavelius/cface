#include "amap.h"
#include "command.h"
#include "crt.h"
#include "draw.h"

static void draw_icon(int& x, int& y, int x0, int x2, int* max_width, int& w, const char* id) {
	static amap<const char*, draw::surface> source;
	auto p = source.find(id);
	if(!p) {
		char temp[260];
		p = source.add(id);
		memset(p, 0, sizeof(*p));
		p->read(szurl(temp, "art/icons", id, "png"), 0, -32);
	}
	auto dy = draw::texth();
	w = p->width;
	if(x + w > x2) {
		if(max_width)
			*max_width = imax(*max_width, x - x0);
		x = x0;
		y += draw::texth();
	}
	draw::blit(*draw::canvas, x, y + dy - p->height, w, p->height, ImageTransparent, *p, 0, 0);
}

COMMAND(app_initialize) {
	draw::set(draw_icon);
}