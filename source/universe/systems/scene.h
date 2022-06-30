#pragma once

#include "../system.h"

namespace flame
{
	struct DrawData;

	/// Reflect
	struct sScene : System
	{
		OctNode* octree = nullptr;

		virtual void generate_nav_mesh() = 0;
		virtual std::vector<vec3> query_nav_path(const vec3& start, const vec3& end) = 0;
		virtual void get_debug_draw(DrawData& draw_data) = 0;

		struct Instance
		{
			virtual sScenePtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sScenePtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
