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
			struct autocomplete* records;
			unsigned		align;
			bool			readonly;
			bool			update_records;
			//
			textedit(char* string, unsigned maxlenght, bool select_text);
			//
			void			clear();
			void			background(rect& rc) override;
			virtual void	cashing(rect rc);
			void			correct();
			bool			editing(rect rc);
			void			ensurevisible(int linenumber);
			int				getrecordsheight() const;
			int				hittest(rect rc, point pt, unsigned state) const;
			void			inputsymbol(int id, int symbol) override;
			void			invalidate() override;
			bool			isshowrecords() const { return getrecordsheight() != 0; }
			void			keybackspace(int id) override;
			void			keydelete(int id) override;
			void			keydown(int id) override;
			void			keyend(int id) override;
			void			keyhome(int id) override;
			void			keyleft(int id) override;
			void			keyright(int id) override;
			void			keyup(int id) override;
			int				lineb(int index) const;
			int				linee(int index) const;
			int				linen(int index) const;
			void			left(bool shift, bool ctrl);
			int				getbegin() const;
			int				getend() const;
			point			getpos(rect rc, int index, unsigned state) const;
			void			paste(const char* string);
			void			redraw(rect rc) override;
			void			right(bool shift, bool ctrl);
			void			select(int index, bool shift);
			unsigned		select_all(bool run);
			void			updaterecords();
		protected:
			int				cashed_width;
			int				cashed_string;
			int				cashed_origin;
		};
	}
}