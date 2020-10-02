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
	};

	struct cTerrainPrivate : cTerrainBridge // R ~ on_*
	{
		int height_map_id = -1;

		std::string height_map_name;
		Vec2u size = Vec2u(64);
		Vec3f extent = Vec3f(100.f);
		float tess_levels = 32.f;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		const char* get_height_map() const override { return height_map_name.c_str(); }
		void set_height_map(const std::string& name);
		Vec2u get_size() const override { return size; }
		void set_size(const Vec2u& s) override;
		Vec3f get_extent() const override { return extent; }
		void set_extent(const Vec3f& e) override;
		float get_tess_levels() const override { return tess_levels; }
		void set_tess_levels(float l) override;

		void on_gain_canvas();

		void draw(graphics::Canvas* canvas); // R
	};

	inline void cTerrainBridge::set_height_map(const char* name)
	{
		((cTerrainPrivate*)this)->set_height_map(name);
	}
}
