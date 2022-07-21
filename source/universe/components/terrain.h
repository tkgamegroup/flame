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
		vec3 extent = vec3(256.f, 128.f, 256.f);
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

		/// Reflect
		std::filesystem::path splash_map_name;
		/// Reflect
		virtual void set_splash_map_name(const std::filesystem::path& name) = 0;

		/// Reflect
		std::filesystem::path material_name;
		/// Reflect
		virtual void set_material_name(const std::filesystem::path& name) = 0;

		/// Reflect
		bool use_grass_field = false;
		/// Reflect
		uint grass_field_tess_level = 64;
		/// Reflect
		float grass_normal_limit = 0.9f;

		graphics::ImagePtr height_map = nullptr;
		graphics::ImagePtr normal_map = nullptr;
		graphics::ImagePtr tangent_map = nullptr;
		graphics::ImagePtr splash_map = nullptr;
		graphics::MaterialPtr material = nullptr;
		int material_res_id = -1;
		int instance_id = -1;
		int grass_field_id = -1;

		virtual void update_normal_map() = 0;

		struct Create
		{
			virtual cTerrainPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
