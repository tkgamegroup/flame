#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cLayout : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		ElementLayoutType type = ElementLayoutVertical;
		// Reflect
		virtual void set_type(ElementLayoutType type) = 0;

		// Reflect
		vec4 padding = vec4(4.f); // L T R B
		// Reflect
		virtual void set_padding(const vec4& padding) = 0;

		// Reflect
		float item_spacing = 4.f;
		// Reflect
		virtual void set_item_spacing(float spacing) = 0;

		// Reflect
		bool auto_width = true;
		// Reflect
		virtual void set_auto_width(bool auto_width) = 0;
		// Reflect
		bool auto_height = true;
		// Reflect
		virtual void set_auto_height(bool auto_height) = 0;

		struct Create
		{
			virtual cLayoutPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
