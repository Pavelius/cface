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
			static unsigned backspace(control* source, bool run);
			virtual void	cashing(rect rc);
			void			correct();
			static unsigned delsym(control* source, bool run);
			bool			editing(rect rc);
			static unsigned end(control* source, bool run);
			void			ensurevisible(int linenumber);
			command*		getcommands() const override;
			static unsigned home(control* source, bool run);
			int				hittest(rect rc, point pt, unsigned state) const;
			void			invalidate() override;
			int				lineb(int index) const;
			int				linee(int index) const;
			int				linen(int index) const;
			void			left(bool shift, bool ctrl);
			static unsigned	left(control* source, bool run);
			static unsigned leftc(control* source, bool run);
			static unsigned lefts(control* source, bool run);
			static unsigned leftcs(control* source, bool run);
			int				getbegin() const;
			int				getend() const;
			point			getpos(rect rc, int index, unsigned state) const;
			void			paste(const char* string);
			void			redraw(rect rc) override;
			void			right(bool shift, bool ctrl);
			static unsigned right(control* source, bool run);
			static unsigned rights(control* source, bool run);
			static unsigned rightc(control* source, bool run);
			static unsigned rightcs(control* source, bool run);
			void			select(int index, bool shift);
			static unsigned symbol(control* source, bool run);
		protected:
			int				cashed_width;
			int				cashed_string;
			int				cashed_origin;
		};
	}
}