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
			static command	commands[];
			list();
			virtual void	background(rect& rc) override;
			virtual void	collapse(int row) {} // expanding 'row'
			void			correction();
			unsigned		dblclick(bool run);
			unsigned		down(bool run);
			unsigned		end(bool run);
			void			ensurevisible(); // ensure that current selected item was visible on screen if current 'count' is count of items per line
			virtual void	expand(int row, int level) {} // expanding 'row' with 'level' = get(row, TreeLevel) or 0
			virtual int		getlevel(int row) const { return 0; } // 0 is no treemark, 1+ level of treemark (where 1 is root level).
			int				getlastchild(int row) const; // get last element of same level
			int				getparent(int row) const; // get parent index of 'row'
			int				getroot(int row) const;
			unsigned		home(bool run);
			virtual bool	isgroup(int row) const { return false; }
			bool			isopen(int row); // Is 'row' open ?
			unsigned		pagedown(bool run);
			unsigned		pageup(bool run);
			void			redraw(rect rc) override;
			virtual bool	selecting(rect rc, int index, point mouse) { return true; }
			void			select(int index);
			void			toggle(int index);
			virtual void	row(rect rc, int index); // Draw single row - part of list
			unsigned		up(bool run);
			unsigned		wheeldown(bool run);
			unsigned		wheelup(bool run);
		};
		//
		struct listfilter : list
		{
			const char*		filter;
			listfilter();
			unsigned		update(bool run);
		};
		// Abstract element's collection presentation
		struct listdata : list
		{
			const void**	source; // References to objects
			const xsfield*	fields; // Metadata of objects
			const char*		name; // Which field used to presentation
			listdata(const void** source, unsigned count, const xsfield* fields, const char* name);
			void			row(rect rc, int id) override;
		};
	}
}