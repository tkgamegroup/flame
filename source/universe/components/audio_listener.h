#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cAudioListener : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		struct Create
		{
			virtual cAudioListenerPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}

