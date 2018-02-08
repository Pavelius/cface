#include "bsreq.h"
#include "draw_control.h"

using namespace draw;

bsreq control_type[] = {
	BSREQ(control, dock, number_type),
	BSREQ(control, focused, number_type),
	BSREQ(control, disabled, number_type),
	BSREQ(control, show_background, number_type),
	BSREQ(control, show_border, number_type),
	BSREQ(control, show_toolbar, number_type),
	{}
};