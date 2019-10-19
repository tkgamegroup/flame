#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct ListenerHub
	{
		std::vector<std::unique_ptr<Closure<void(void* c)>>> listeners;
	};
}
