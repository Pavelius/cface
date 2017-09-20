#include "draw.h"
#include "pointl.h"

#pragma once

namespace draw
{
	namespace controls
	{
		struct scrollable : public control // Abstract scrollable element. Scroll apear automatically if needed.
		{
			pointl			origin;
			pointl			maximum;
			point			wheels;
			scrollable();
			rect			centerview(rect rc);
			virtual void	invalidate();
			void			nonclient(rect rc) override;
		};
	}
}