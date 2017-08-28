#include "crt.h"
#include "win.h"

void dlgmsg(const char* title, const char* text)
{
	MessageBoxA(GetActiveWindow(), text, title, 0);
}

void dlgerr(const char* title, const char* format, ...)
{
	char temp[4096];
	szprintv(temp, format, xva_start(format));
	MessageBoxA(GetActiveWindow(), temp, title, MB_OK|MB_ICONERROR);
}

bool dlgask(const char* title, const char* text)
{
	return (MessageBoxA(GetActiveWindow(), text, title, MB_YESNO|MB_ICONQUESTION))==IDYES;
}