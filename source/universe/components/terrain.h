#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cTerrain : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		vec3 extent = vec3(200.f, 50.f, 200.f);
		/// Reflect
		virtual void set_extent(const vec3& extent) = 0;

		/// Reflect
		uvec2 blocks = uvec2(32);
		/// Reflect
		virtual void set_blocks(const uvec2& blocks) = 0;

		/// Reflect
		uint tess_level = 8.f;
		/// Reflect
		virtual void set_tess_level(uint tess_level) = 0;

		/// Reflect
		std::filesystem::path height_map_name;
		/// Reflect
		virtual void set_height_map_name(const std::filesystem::path& name) = 0;

		std::unique_ptr<graphics::Image> height_map;
		graphics::MaterialPtr material = nullptr;
		int material_res_id = -1;
		int instance_id = -1;

		// { height, normal, tangent } as array
		std::unique_ptr<graphics::Image> textures;

		struct Create
		{
			virtual cTerrainPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
