#pragma once

#include "foundation.h"

namespace flame
{
	extern uint frames;
	extern uint64 last_time;
	extern float delta_time;
	extern float total_time;
	extern uint fps;
	extern float fps_delta;

	extern std::vector<std::unique_ptr<NativeWindowPrivate>> windows;
}
