#include "draw_scrollable.h"

#pragma once

namespace draw
{
	namespace controls
	{
		struct textedit : public scrollable
		{
			char*			string;
			unsigned		maxlenght;
			int				p1, p2;
			rect			rctext, rcclient;
			struct listfilter* records;
			unsigned		align;
			bool			readonly;
			bool			update_records;
			static command	commands[];
			//
			textedit(char* string, unsigned maxlenght);
			//
			void			clear();
			void			background(rect& rc) override;
			unsigned		backspace(bool run);
			virtual void	cashing(rect rc);
			void			correct();
			unsigned		delsym(bool run);
			unsigned		down(bool run);
			unsigned		downs(bool run);
			bool			editing(rect rc);
			unsigned		end(bool run);
			void			ensurevisible(int linenumber);
			command*		getcommands() const override { return commands; }
			int				getrecordsheight() const;
			unsigned		home(bool run);
			int				hittest(rect rc, point pt, unsigned state) const;
			void			invalidate() override;
			bool			isshowrecords() const { return getrecordsheight() != 0; }
			int				lineb(int index) const;
			int				linee(int index) const;
			int				linen(int index) const;
			void			left(bool shift, bool ctrl);
			unsigned		left(bool run);
			unsigned		leftc(bool run);
			unsigned		lefts(bool run);
			unsigned		leftcs(bool run);
			int				getbegin() const;
			int				getend() const;
			point			getpos(rect rc, int index, unsigned state) const;
			void			paste(const char* string);
			void			redraw(rect rc) override;
			void			right(bool shift, bool ctrl);
			unsigned		right(bool run);
			unsigned		rights(bool run);
			unsigned		rightc(bool run);
			unsigned		rightcs(bool run);
			void			select(int index, bool shift);
			unsigned		symbol(bool run);
			unsigned		up(bool run);
			void			updaterecords();
			unsigned		ups(bool run);
		protected:
			int				cashed_width;
			int				cashed_string;
			int				cashed_origin;
		};
	}
}