#include "point.h"
#include "rect.h"

#pragma once

enum drawable_flags {
	DrawableModeMask = 0x1F
};

struct drawable {
	struct plugin {
		plugin*			next;
		static plugin*	first;
		plugin();
		virtual void	selecting(drawable** result, unsigned flags) {}
		virtual void	updating() {}
	};
	virtual int			getcursor() const { return 0; } // Get cursor index when over this drawable
	virtual int			getid() const { return 0; } // Context of this drawable
	virtual unsigned	getfps() const { return 20; }
	virtual rect		getrect() const = 0;
	virtual point		getzpos() const = 0;
	virtual bool		hittest(point position, point mouse) const { return false; }
	virtual bool		isvisible() const { return true; }
	virtual bool		isvisibleactive() const { return false; } // Drawable visible only when active.
	virtual void		painting(point position) const = 0; // How to paint drawable.
	virtual int			priority() const { return 100; }// Priority for z-order sortering (lesser was be most visible). If there is two drawable in same position.
	virtual void		update() {}// Update some states (animation, frame etc)
};
int						dwclipping(drawable** da, rect screen, point camera); // In place clipping drawable by draft mode.
drawable*				dwfind(drawable** da, rect screen, point camera, point hitpoint); // Make hittest by previously Z-Ordered or clipped arrays of drawables.
drawable**				dworder(drawable** da, int count); // Prepare right z-order of each drawable for painting.
void					dwpaint(drawable** da, rect screen, point camera); // Calling painting() for each drawable in list.
drawable**				dwselect(drawable** da, unsigned flags = 0);
void					dwupdate();