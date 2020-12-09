#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cController : Component
	{
		inline static auto type_name = "flame::cController";
		inline static auto type_hash = ch(type_name);

		cController() :
			Component(type_name, type_hash)
		{
		}

		virtual float get_radius() const = 0;
		virtual void set_radius(float r) = 0;
		virtual float get_height() const = 0;
		virtual void set_height(float h) = 0;

		virtual void move(const vec3& disp) = 0;

		FLAME_UNIVERSE_EXPORTS static cController* create();
	};
}
