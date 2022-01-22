#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sScene : System
	{
		OctNode* octree = nullptr;

		struct Instance
		{
			virtual sScenePtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sScenePtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
