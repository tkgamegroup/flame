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
		void set_color_map(const char* name) override;
	};

	struct cTerrainPrivate : cTerrainBridge // R ~ on_*
	{
		Vec2u blocks = Vec2u(64);
		Vec3f scale = Vec3f(100.f);
		uint tess_levels = 32.f;

		std::string height_map_name;
		std::string normal_map_name;
		std::string color_map_name;
		int height_map_id = -1;
		int normal_map_id = -1;
		int color_map_id = -1;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		Vec2u get_blocks() const override { return blocks; }
		void set_blocks(const Vec2u& b) override;
		Vec3f get_scale() const override { return scale; }
		void set_scale(const Vec3f& s) override;
		uint get_tess_levels() const override { return tess_levels; }
		void set_tess_levels(uint l) override;

		const char* get_height_map() const override { return height_map_name.c_str(); }
		void set_height_map(const std::string& name);
		const char* get_normal_map() const override { return normal_map_name.c_str(); }
		void set_normal_map(const std::string& name);
		const char* get_color_map() const override { return color_map_name.c_str(); }
		void set_color_map(const std::string& name);

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

	inline void cTerrainBridge::set_color_map(const char* name)
	{
		((cTerrainPrivate*)this)->set_color_map(name);
	}
}
