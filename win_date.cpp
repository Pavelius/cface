#include "win.h"
#include "crt.h"

//extern "C" unsigned clock()
//{
//	return GetTickCount();
//}

unsigned getdate()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	return mkdate(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
}

extern "C" void sleep(unsigned seconds)
{
	Sleep(seconds);
}