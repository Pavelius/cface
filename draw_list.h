#include "draw_control.h"

#pragma once

struct xsfield;

namespace draw
{
	namespace controls
	{
		// Basic list class
		struct list : control
		{
			int				origin, maximum, current;
			int				maximum_width, origin_width;
			int				lines_per_page, pixels_per_line;
			bool			show_grid_lines;
			bool			hilite_rows;
			list();
			virtual void	background(rect& rc) override;
			virtual void	collapse(int row) {} // expanding 'row'
			void			correction();
			void			ensurevisible(); // ensure that current selected item was visible on screen if current 'count' is count of items per line
			virtual void	expand(int row, int level) {} // expanding 'row' with 'level' = get(row, TreeLevel) or 0
			virtual int		getlevel(int row) const { return 0; } // 0 is no treemark, 1+ level of treemark (where 1 is root level).
			int				getlastchild(int row) const; // get last element of same level
			int				getparent(int row) const; // get parent index of 'row'
			int				getroot(int row) const;
			void			keydown(int id) override;
			void			keyend(int id) override;
			void			keyenter(int id) override;
			void			keyhome(int id) override;
			void			keypagedown(int id) override;
			void			keypageup(int id) override;
			void			keyup(int id) override;
			void			mouseleftdbl(point position, int id) override;
			void			mousewheel(point position, int id, int step) override;
			virtual bool	isgroup(int row) const { return false; }
			bool			isopen(int row); // Is 'row' open ?
			void			redraw(rect rc) override;
			virtual bool	selecting(rect rc, int index, point mouse) { return true; }
			void			select(int index);
			void			toggle(int index);
			virtual void	row(rect rc, int index); // Draw single row - part of list
			void			updaterowheight();
		};
		// Abstract element's collection presentation
		struct listview : list
		{
			const void**	source; // References to objects
			const xsfield*	fields; // Metadata of objects
			const xsfield*	requisit; // Which field used to presentation
			listview(const void** source, unsigned count, const xsfield* fields, const char* name);
			int				find(const char* name) const;
			const char*		getname(int index) const;
			void			row(rect rc, int id) override;
			void			setpresetation(const char* name);
		};
		// Edit string collections
		struct autocomplete : listview
		{
			const void*		source[50];
			autocomplete(const xsfield* fields = 0, const char* name = "name") : listview(source, 0, fields, name), filter(0) {}
			const char*		filter;
			virtual void	update() = 0;
		};
	}
}