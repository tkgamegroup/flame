#pragma once

#include "../component.h"

namespace flame
{
	struct cLight : Component
	{
		inline static auto type_name = "flame::cLight";
		inline static auto type_hash = ch(type_name);

		cLight() :
			Component(type_name, type_hash)
		{
		}

		virtual LightType get_type() const = 0;
		virtual void set_type(LightType t) = 0;

		virtual vec3 get_color() const = 0;
		virtual void set_color(const vec3& c) = 0;

		virtual bool get_cast_shadow() const = 0;
		virtual void set_cast_shadow(bool v) = 0;

		virtual mat4 get_shadow_mat(uint idx) const = 0;

		FLAME_UNIVERSE_EXPORTS static cLight* create();
	};
}
