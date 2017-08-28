//Copyright 2015 by Pavel Chistyakov
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "pointl.h"
#include "point.h"
#include "widget.h"

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