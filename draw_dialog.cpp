#include "draw.h"

static void(*current_execute)();

static void callback_setfocus()
{
	draw::setfocus(hot::param);
}

bool draw::dodialog(int id)
{
	switch(id)
	{
	case InputExecute:
		if(current_execute)
		{
			auto temp_execute = current_execute;
			current_execute = 0;
			temp_execute();
			return true;
		}
		break;
	case KeyTab:
	case KeyTab | Shift:
	case KeyTab | Ctrl:
	case KeyTab | Ctrl | Shift:
		id = getnext(getfocus(), id);
		if(id)
			setfocus(id);
		return true;
	}
	return false;
}

void draw::invoke(void(*proc)())
{
	draw::execute(InputExecute);
	current_execute = proc;
}

void draw::focusing(int id, unsigned& flags, rect rc)
{
	if(flags&Disabled)
		return;
	addelement(id, rc);
	if(!getfocus())
		setfocus(id);
	else if(getfocus() == id)
		flags |= Focused;
	else if(area(rc) == AreaHilitedPressed && hot::key == MouseLeft && hot::pressed)
	{
		invoke(callback_setfocus);
		hot::param = id;
	}
}

int	draw::button(int x, int y, int width, int id, const char* label, bool disabled, int key, const char* tips)
{
	setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
	unsigned flags = 0;
	if(disabled)
		flags |= Disabled;
	focusing(id, flags, rc);
	if(buttonh({x, y, x + width, rc.y2},
		ischecked(flags), isfocused(flags), isdisabled(flags), true,
		label, KeyEnter, false, tips))
	{
		draw::execute(id);
	}
	return rc.height() + metrics::padding * 2;
}