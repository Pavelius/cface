#include "widget_scrollable.h"

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
			rect			rctext;
			struct listfilter* records;
			unsigned		align;
			bool			readonly;
			bool			update_records;
			//
			textedit(char* string, unsigned maxlenght);
			//
			void			clear();
			void			background(rect& rc) override;
			virtual void	cashing(rect rc);
			void			correct();
			bool			editing(rect rc);
			void			ensurevisible(int linenumber);
			bool			keyinput(int id) override;
			int				hittest(rect rc, point pt, unsigned state) const;
			void			invalidate() override;
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
		protected:
			int				cashed_width;
			int				cashed_string;
			int				cashed_origin;
		};
	}
}