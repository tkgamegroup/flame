#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cTileMap : Component
	{
		struct Sample
		{
			uint height : 5;
			bool is_half : 1;
			uint slope : 2;
		};

		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		vec3 extent = vec3(64.f, 4.f, 64.f);
		// Reflect
		virtual void set_extent(const vec3& extent) = 0;

		// Reflect
		uvec3 blocks = uvec3(32, 2, 32);
		// Reflect
		virtual void set_blocks(const uvec3& blocks) = 0;

		// Reflect
		std::filesystem::path tiles_path;
		// Reflect
		virtual void set_tiles_path(const std::filesystem::path& path) = 0;

		// Reflect
		std::vector<uint> samples;
		// Reflect
		virtual void set_sample(uint idx, uint v) = 0;

		struct Create
		{
			virtual cTileMapPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
