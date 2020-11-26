#pragma once

#include <flame/universe/components/terrain.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;

	struct cTerrainBridge : cTerrain
	{
		void set_height_map(const char* name) override;
		void set_normal_map(const char* name) override;
		void set_material_name(const char* name) override;
	};

	struct cTerrainPrivate : cTerrainBridge // R ~ on_*
	{
		uvec2 blocks = uvec2(64);
		vec3 scale = vec3(100.f);
		uint tess_levels = 32.f;

		std::string height_map_name;
		std::string normal_map_name;
		std::string material_name;
		int height_map_id = -1;
		int normal_map_id = -1;
		int material_id = -1;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		uvec2 get_blocks() const override { return blocks; }
		void set_blocks(const uvec2& b) override;
		vec3 get_scale() const override { return scale; }
		void set_scale(const vec3& s) override;
		uint get_tess_levels() const override { return tess_levels; }
		void set_tess_levels(uint l) override;

		const char* get_height_map() const override { return height_map_name.c_str(); }
		void set_height_map(const std::string& name);
		const char* get_normal_map() const override { return normal_map_name.c_str(); }
		void set_normal_map(const std::string& name);
		const char* get_material_name() const override { return material_name.c_str(); }
		void set_material_name(const std::string& name);

		void on_gain_canvas();

		void draw(graphics::Canvas* canvas); // R
	};

	inline void cTerrainBridge::set_height_map(const char* name)
	{
		((cTerrainPrivate*)this)->set_height_map(name);
	}

	inline void cTerrainBridge::set_normal_map(const char* name)
	{
		((cTerrainPrivate*)this)->set_normal_map(name);
	}

	inline void cTerrainBridge::set_material_name(const char* name)
	{
		((cTerrainPrivate*)this)->set_material_name(name);
	}
}
