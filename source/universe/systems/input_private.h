#pragma once

#include "input.h"

namespace flame
{
	struct sInputPrivate : sInput
	{
		vec2 drag_pos = vec2(0.f);
		bool mbtn_temp[MouseButton_Count] = {};
		vec2 mpos_temp = vec2(0.f);
		int mscr_temp = 0;
		bool kbtn_temp[KeyboardKey_Count] = {};

		NativeWindowPtr bound_window = nullptr;

		sInputPrivate();
		~sInputPrivate();

		void bind_window(NativeWindowPtr w) override;
		void update() override;
	};
}
