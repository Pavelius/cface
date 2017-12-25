#include "draw_control.h"
#include "widget.h"

#pragma once

namespace draw
{
	namespace controls
	{
		struct form : control
		{
			typedef int		(form::*proc)(int x, int y, int width, unsigned flags, const widget& e);
			struct control_i
			{
				const char*	name;
				form::proc	proc;
			};
			const bsreq*	fields;
			void*			source;
			static control_i renders[];
			//
			form(const bsreq* fields, void* source) : fields(fields), source(source) {}
			//
			bool			addbutton(rect& rc, const char* t1, int k1, const char* tt1);
			int				addbutton(rect& rc, const char* t1, int k1, const char* tt1, const char* t2, int k2, const char* tt2);
			int				button(int x, int y, int width, unsigned flags, const widget& e);
			int				check(int x, int y, int width, unsigned flags, const widget& e);
			int				decoration(int x, int y, int width, unsigned flags, const widget& e);
			int				element(int x, int y, int width, unsigned flags, const widget& e);
			bool			isvisible(const widget& e) const { return true; }
			int				field(int x, int y, int width, unsigned flags, const widget& e);
			void			focusing(const rect& rc, unsigned& flags, const widget& e);
			unsigned		getflags(const widget& e, unsigned flags) const;
			int				getdata(const widget& e);
			char*			getdata(char* result, const widget& e, bool to_buffer = false);
			const bsreq*	getmeta() const override { return fields; }
			void*			getobject() override { return source; }
			int				group(int x, int y, int width, unsigned flags, const widget& e);
			int				header(int& x, int y, int& width, unsigned flags, const char* label, int title);
			int				horizontal(int x, int y, int width, unsigned flags, const widget& e);
			int				radio(int x, int y, int width, unsigned flags, const widget& e);
			int				renderno(int x, int y, int width, unsigned flags, const widget& e) { return 0; }
			void			setdata(const widget& e, int value, bool instant = false);
			static void		setfocus(const widget& value);
			int				tabs(int x, int y, int width, unsigned flags, const widget& e);
			int				vertical(int x, int y, int width, unsigned flags, const widget& e);
			int				view(int x, int y, int width, const widget* source);
		};
	}
}