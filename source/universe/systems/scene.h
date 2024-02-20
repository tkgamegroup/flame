#pragma once

#include "../system.h"

namespace flame
{
	struct DrawData;

	// Reflect ctor
	struct sScene : System
	{
		EntityPtr first_node = nullptr;
		EntityPtr first_element = nullptr;
		OctNode* octree = nullptr;

		// Reflect
		virtual void				navmesh_generate(const std::vector<EntityPtr>& nodes, float agent_radius, float agent_height, float walkable_climb, float walkable_slope_angle, float cell_size = 0.3f, float cell_height = 0.2f, float tile_size = 48.f) = 0;
		// Reflect
		virtual void				navmesh_clear() = 0;
		// Reflect
		virtual bool				navmesh_nearest_point(const vec3& center, const vec3& ext, vec3& res) = 0;
		// Reflect
		virtual std::vector<vec3>	navmesh_query_path(const vec3& start, const vec3& end, uint max_smooth = 2048) = 0;
		// Reflect
		virtual bool				navmesh_check_free_space(const vec3& pos, float radius) = 0;
		// Reflect
		virtual std::vector<vec3>	navmesh_get_mesh() = 0;
		// Reflect
		virtual void				navmesh_save(const std::filesystem::path& filename) = 0;
		// Reflect
		virtual void				navmesh_load(const std::filesystem::path& filename) = 0;
		// Reflect
		virtual void draw_debug_primitives() = 0;

		struct Instance
		{
			virtual sScenePtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sScenePtr operator()(WorldPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
