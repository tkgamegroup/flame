#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cNavAgent : Component
	{
		/// Reflect
		float radius = 0.3f;
		/// Reflect
		float height = 2.f;

		struct Create
		{
			virtual cNavAgentPtr operator()() = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
