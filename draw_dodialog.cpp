#include "draw.h"

static void(*current_execute)();

static int current_focus; // Current focus element

static void callback_setfocus()
{
	draw::setfocus(hot::param, true);
}

int draw::getfocus()
{
	return current_focus;
}

void draw::setfocus(int value, bool instant)
{
	if(instant)
		current_focus = value;
	else
	{
		execute(callback_setfocus);
		hot::param = value;
	}
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
			setfocus(id, true);
		return true;
	}
	return false;
}

void draw::execute(void(*proc)())
{
	draw::execute(InputExecute);
	current_execute = proc;
}