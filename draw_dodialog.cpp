#include "draw.h"

static void(*current_execute)();

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

void draw::execute(void(*proc)())
{
	draw::execute(InputExecute);
	current_execute = proc;
}