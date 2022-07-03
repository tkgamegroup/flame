#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cSdf : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		int instance_id = -1;

		struct Create
		{
			virtual cSdfPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
