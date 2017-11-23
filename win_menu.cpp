#include "crt.h"
#include "draw.h"
#include "menu.h"
#include "win.h"

using namespace draw;

menu::menu() : count(0), separator(false)
{
	handle = CreatePopupMenu();
}

menu::~menu()
{
	DestroyMenu(handle);
}

static int add_menu(void* parent, unsigned& count, const char* label, unsigned data, menu* child, bool disabled, bool checked, bool radio)
{
	MENUITEMINFO	mi = {0};
	char			temp[260];
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
	mi.fType = MFT_STRING;
	if(radio)
		mi.fType |= MFT_RADIOCHECK;
	temp[0] = 0;
	if(label)
		zcpy(temp, label, sizeof(temp)-1);
	szupper(temp, 1);
	mi.dwTypeData = temp;
	mi.wID = ++count;
	mi.hSubMenu = child;
	if(!disabled)
		mi.fState = MFS_ENABLED;
	else
		mi.fState = MFS_DISABLED | MFS_GRAYED;
	if(checked)
		mi.fState |= MFS_CHECKED;
	mi.dwItemData = data;
	return InsertMenuItemA(parent, mi.wID, 0, &mi);
}

static void add_separator(menu& e)
{
	if(e.separator)
	{
		AppendMenuA(e.handle, MF_SEPARATOR, -1, 0);
		e.count++;
	}
	e.separator = false;
}

void menu::add(const char* label, unsigned data, bool disabled, bool checked)
{
	add_separator(*this);
	add_menu(handle, count, label, data, 0, disabled, checked, false);
}

void menu::addseparator()
{
	if(!separator && count)
		separator = true;
}

void menu::add(const char* id, draw::control* object)
{
	auto pe = object->getcommands()->find(id);
	if(pe)
	{
		char temp[512];
		zcpy(temp, pe->label, sizeof(temp) - 1);
		auto ps = zend(temp);
		for(auto key : pe->key)
		{
			if(!key)
				break;
			if(ps[0]==0)
				zcat(ps, "\t");
			else
				zcat(ps, ", ");
			key2str(zend(ps), key);
		}
		add(temp, (unsigned)pe, pe->type(object, false) == Disabled, false);
	}
}

unsigned get_menu_data(void* handle, unsigned index)
{
	MENUITEMINFO mi = {0};
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_DATA;
	if(!GetMenuItemInfoA(handle, index-1, 1, &mi))
		return 0;
	return mi.dwItemData;
}

unsigned menu::choose(int x, int y)
{
	if(!handle)
		return 0;
	HWND hwnd = GetActiveWindow();
	if(!hwnd)
		return 0;
	POINT pt; pt.x = x; pt.y = y;
	ClientToScreen(hwnd, &pt);
	// show menu
	auto result = TrackPopupMenuEx(handle, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, hwnd, 0);
	if(result)
		result = get_menu_data(handle, result);
	DestroyMenu(handle);
	return result;
}