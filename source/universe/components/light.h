#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cLight : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		LightType type = LightDirectional;
		/// Reflect
		virtual void set_type(LightType type) = 0;
		/// Reflect
		vec4 color = vec4(1.f);
		/// Reflect
		virtual void set_color(const vec4& color) = 0;
		/// Reflect
		float range = 10.f;
		/// Reflect
		virtual void set_range(float range) = 0;
		/// Reflect
		bool cast_shadow = false;
		/// Reflect
		virtual void set_cast_shadow(bool cast_shadow) = 0;

		int instance_id = -1;

		struct Create
		{
			virtual cLightPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
