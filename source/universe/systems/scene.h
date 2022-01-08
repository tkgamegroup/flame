#pragma once

#include "../system.h"

namespace flame
{
	struct sScene : System
	{
		virtual ~sScene() {}

		struct Create
		{
			virtual sScenePtr operator()() = 0;
		};
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
