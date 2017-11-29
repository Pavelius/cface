#include "crt.h"
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

int	draw::button(int x, int y, int width, int id, unsigned flags, const char* label, int key, const char* tips)
{
	setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
	focusing(id, flags, rc);
	if(buttonh({x, y, x + width, rc.y2},
		ischecked(flags), isfocused(flags), isdisabled(flags), true,
		label, KeyEnter, false, tips))
	{
		draw::execute(id);
	}
	return rc.height() + metrics::padding * 2;
}

int draw::radio(int x, int y, int width, int id, unsigned flags, const char* label, const char* tips)
{
	draw::state push;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc1.offset(-2);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	decortext(flags);
	focusing(id, flags, rc);
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), width, flags, ":radio");
	bool need_select = false;
	auto a = draw::area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot::key == MouseLeft)
	{
		if(!hot::pressed)
			need_select = true;
	}
	if(isfocused(flags))
	{
		draw::rectx({rc1.x1, rc1.y1, rc1.x2, rc1.y2}, draw::fore);
		if(!isdisabled(flags) && hot::key == KeySpace)
			need_select = true;
	}
	if(need_select)
		execute(id);
	draw::text({rc1.x1 + 2, rc1.y1 + 2, rc1.x2 - 2, rc1.y2 - 2}, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + metrics::padding * 2;
}