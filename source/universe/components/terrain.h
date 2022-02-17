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
		std::filesystem::path texture_name;
		/// Reflect
		virtual void set_texture_name(const std::filesystem::path& texture_name) = 0;

		int instance_id = -1;

		// { height, normal, tangent } as array
		std::unique_ptr<graphics::Image> textures = nullptr;

		struct Create
		{
			virtual cTerrainPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
