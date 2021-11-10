#pragma once

#include "foundation.h"

namespace flame
{
	extern uint frames;
	extern float delta_time;
	extern float total_time;
	extern uint fps;

	extern std::vector<std::unique_ptr<NativeWindowPrivate>> windows;
}
