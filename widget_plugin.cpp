#include "crt.h"
#include "draw.h"
#include "widget.h"

using namespace draw;

widget::plugin* widget::plugin::first;

widget::plugin::plugin(widget::plugin::element* controls) : controls(controls)
{
	seqlink(this);
}

widget::plugin::element* widget::plugin::find(const char* id)
{
	for(auto pm = first; pm; pm = pm->next)
	{
		for(auto p = pm->controls; p->id; p++)
		{
			if(strcmp(p->id, id) == 0)
				return p;
		}
	}
	return 0;
}