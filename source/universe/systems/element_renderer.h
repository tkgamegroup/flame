#pragma once

#include "../system.h"

namespace flame
{
	struct sElementRenderer : System
	{
		struct Create
		{
			virtual sElementRendererPtr operator()(WorldPtr w) = 0;
		};
		FLAME_UNIVERSE_API static Create& create;
	};
}
