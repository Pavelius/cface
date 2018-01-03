#include "draw.h"
#include "draw_control.h"

using namespace draw;

void control::focusing(const rect& rc) {
	unsigned flags = 0;
	if(disabled)
		flags |= Disabled;
	draw::focusing((int)this, flags, rc);
	focused = isfocused(flags);
}

void control::viewf(rect rc, bool show_toolbar) {
	draw::state push;
	struct rect rt = {rc.x1, rc.y1, rc.x2, rc.y1};
	auto commands = getcommands();
	if(show_toolbar && metrics::toolbar && commands && this->show_toolbar) {
		rt.y2 += metrics::toolbar->get(0).sy + 4;
		rc.y1 += rt.height() + metrics::padding;
	}
	enablefocus();
	background(rc);
	focusing(rc);
	prerender();
	enablemouse(rc);
	nonclient(rc);
	if(rt.height())
		toolbar(rt.x1, rt.y1, rt.width(), commands);
}