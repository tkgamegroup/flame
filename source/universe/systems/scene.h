#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sScene : System
	{
		struct Create
		{
			virtual sScenePtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
