#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cLayout : Component
	{
		// Reflect
		ElementLayoutType type = ElementLayoutVertical;
		// Reflect
		virtual void set_type(ElementLayoutType type) = 0;

		// Reflect
		vec4 padding = vec4(4.f);
		// Reflect
		virtual void set_padding(const vec4& padding) = 0;

		// Reflect
		float item_spacing = 4.f;
		// Reflect
		virtual void set_item_spacing(float spacing) = 0;

		// Reflect
		bool auto_width = false;
		// Reflect
		virtual void set_auto_width(bool auto_width) = 0;
		// Reflect
		bool auto_height = false;
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
