#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cCamera : Component
	{
		// Reflect optional_requires=1
		cNodePtr node = nullptr;
		// Reflect optional_requires=1
		cElementPtr element = nullptr;

		// Reflect
		float fovy = 45.f;
		// Reflect
		float zNear = 1.f;
		// Reflect
		float zFar = 1000.f;
		// x / y
		float aspect = 1.f;
		// Reflect
		vec2 pivot = vec2(0.f);
		// Reflect
		vec2 restrict_lt = vec2(0.f);
		// Reflect
		vec2 restrict_rb = vec2(0.f);
		// Reflect
		uint layer = 1;

		mat4 view_mat;
		mat4 view_mat_inv;
		mat4 proj_mat;
		mat4 proj_mat_inv;
		mat4 proj_view_mat;
		mat4 proj_view_mat_inv;
		Frustum frustum;
		vec2 translate_2d;
		vec2 scaling_2d;

		virtual vec2 world_to_screen(const vec3& pos, vec3* out_clip_coord = nullptr) = 0;

		struct Create
		{
			virtual cCameraPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;

		struct List
		{
			virtual const std::vector<cCameraPtr>& operator()() = 0;
		};
		FLAME_UNIVERSE_API static List& list;
	};
}
