#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cLayout : Component
	{
		enum Type
		{
			Free,
			Vertical,
			Horizontal,
			Grid
		};

		// Reflect
		Type type = Free;
		// Reflect
		virtual void set_type(Type type) = 0;

		// Reflect
		float spacing = 4.f;
		// Reflect
		virtual void set_spacing(float spacing) = 0;

		struct Create
		{
			virtual cLayoutPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
