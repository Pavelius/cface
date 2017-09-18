#include "crt.h"
#include "widget.h"

widget::plugin* widget::plugin::first;

widget::plugin::plugin(const char* id, proc render) : id(id), render(render)
{
	seqlink(this);
}

widget::plugin* widget::plugin::find(const char* id)
{
	for(auto p = first; p; p = p->next)
	{
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}