#pragma once
#ifdef USE_PHYSICS_MODULE

#include "../component.h"

namespace flame
{
	struct cCharacterController : Component
	{
		inline static auto type_name = "flame::cCharacterController";
		inline static auto type_hash = ch(type_name);

		cCharacterController() :
			Component(type_name, type_hash)
		{
		}

		virtual float get_radius() const = 0;
		virtual void set_radius(float r) = 0;
		virtual float get_height() const = 0;
		virtual void set_height(float h) = 0;

		virtual float get_static_friction() const = 0;
		virtual void set_static_friction(float v) = 0;

		virtual float get_dynamic_friction() const = 0;
		virtual void set_dynamic_friction(float v) = 0;

		virtual float get_restitution() const = 0;
		virtual void set_restitution(float v) = 0;

		virtual void move(const vec3& disp) = 0;

		FLAME_UNIVERSE_EXPORTS static cCharacterController* create();
	};
}

#endif
