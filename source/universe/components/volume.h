#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cVolume : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		vec3 extent = vec3(32.f);
		// Reflect
		virtual void set_extent(const vec3& extent) = 0;

		// Reflect
		uvec3 blocks = uvec3(8, 1, 8);
		// Reflect
		virtual void set_blocks(const uvec3& blocks) = 0;

		// Reflect
		std::filesystem::path data_map_name;
		// Reflect
		virtual void set_data_map_name(const std::filesystem::path& name) = 0;

		// Reflect
		std::filesystem::path material_name;
		// Reflect
		virtual void set_material_name(const std::filesystem::path& name) = 0;

		// Reflect
		bool cast_shadow = true;
		// Reflect
		virtual void set_cast_shadow(bool v) = 0;

		// Reflect
		bool marching_cubes = true;

		graphics::ImagePtr data_map = nullptr;
		graphics::ImagePtr height_map = nullptr;
		graphics::ImagePtr normal_map = nullptr;
		graphics::ImagePtr tangent_map = nullptr;
		graphics::MaterialPtr material = nullptr;
		int instance_id = -1;
		int material_res_id = -1;

		struct Create
		{
			virtual cVolumePtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
