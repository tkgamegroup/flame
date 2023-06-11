#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cAnimator : Component
	{
		// Reflect
		std::vector<Modifier> modifiers;
		// Reflect
		virtual void set_modifiers(const std::vector<Modifier>& modifiers) = 0;

		struct Create
		{
			virtual cAnimatorPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
