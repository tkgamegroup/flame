#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cDirLight : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		vec4 color = vec4(1.f);
		/// Reflect
		virtual void set_color(const vec4& color) = 0;
		/// Reflect
		bool cast_shadow = false;
		/// Reflect
		virtual void set_cast_shadow(bool cast_shadow) = 0;

		int instance_id = -1;

		struct Create
		{
			virtual cDirLightPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
