#pragma once

#include "../component.h"

namespace flame
{
	struct cCamera : Component
	{
		inline static auto type_name = "flame::cCamera";
		inline static auto type_hash = ch(type_name);

		cCamera() :
			Component(type_name, type_hash)
		{
		}

		virtual bool get_current() const = 0;
		virtual void set_current(bool v) = 0;

		virtual void get_points(vec3* dst, float n = -1.f, float f = -1.f) = 0;
		virtual Frustum get_frustum(float n = -1.f, float f = -1.f) = 0;

		virtual vec3 screen_to_world(const uvec2& pos) = 0;
		virtual uvec2 world_to_screen(const vec3& pos) = 0;

		FLAME_UNIVERSE_EXPORTS static cCamera* create(void* parms = nullptr);
	};
}
