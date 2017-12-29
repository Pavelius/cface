#include "bsreq.h"
#include "widget.h"

using namespace draw;

bsreq widget_type[] = {
	BSREQ(widget, id, text_type),
	BSREQ(widget, flags, number_type),
	BSREQ(widget, label, text_type),
	BSREQ(widget, title, number_type),
	BSREQ(widget, width, number_type),
	BSREQ(widget, height, number_type),
	BSREQ(widget, childs, widget_type),
	{0}
};