#include "draw_control.h"
#include "widget.h"

#pragma once

namespace draw
{
	namespace controls
	{
		struct form : control
		{
			bool			addbutton(rect& rc, const char* t1, int k1, const char* tt1);
			int				addbutton(rect& rc, const char* t1, int k1, const char* tt1, const char* t2, int k2, const char* tt2);
			int				button(int x, int y, int width, const widget& e);
			int				check(int x, int y, int width, const widget& e);
			int				decoration(int x, int y, int width, const widget& e);
			int				element(int x, int y, int width, const widget& e);
			int				field(int x, int y, int width, const widget& e);
			void			focusing(const rect& rc, unsigned& flags, const widget& e);
			unsigned		getflags(const widget& e) const;
			int				getdata(const widget& e) { return 0; }
			char*			getdata(char* result, const widget& e) { return result; }
			int				group(int x, int y, int width, const widget& e);
			int				header(int& x, int y, int& width, unsigned flags, const char* label, int title);
			int				horizontal(int x, int y, int width, const widget& e);
			int				radio(int x, int y, int width, const widget& e);
			void			setdata(const widget& e, int value) { }
			int				tabs(int x, int y, int width, const widget& e);
			int				vertical(int x, int y, int width, const widget& e);
		};
	}
}