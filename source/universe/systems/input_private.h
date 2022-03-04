#pragma once

#include "input.h"

namespace flame
{
	struct sInputPrivate : sInput
	{
		vec2 drag_pos = vec2(0.f);
		bool mbtn_temp[MouseButton_Count] = {};
		vec2 mpos_temp = vec2(0.f);
		bool kbtn_temp[KeyboardKey_Count] = {};

		sInputPrivate();
		~sInputPrivate();

		void update() override;
	};
}
