#pragma once

#include "../system.h"

namespace flame
{
	struct sElementRenderer : System
	{
		struct Create
		{
			virtual sElementRendererPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Create& create;
	};
}
