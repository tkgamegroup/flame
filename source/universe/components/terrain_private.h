#pragma once

#include "terrain.h"

namespace flame
{
	struct cTerrainPrivate : cTerrain
	{
		uvec2 blocks = uvec2(64);
		uint tess_levels = 32.f;

		std::string height_map_name;
		std::string normal_map_name;
		std::string material_name;

		cNodePrivate* node = nullptr;
		void* drawer = nullptr;
		sRendererPrivate* s_renderer = nullptr;

		graphics::Image* height_texture = nullptr;
		graphics::Image* normal_texture = nullptr;
		graphics::Material* material = nullptr;
		int height_map_id = -1;
		int normal_map_id = -1;
		int material_id = -1;

		uvec2 get_blocks() const override { return blocks; }
		void set_blocks(const uvec2& b) override;
		uint get_tess_levels() const override { return tess_levels; }
		void set_tess_levels(uint l) override;

		const char* get_height_map() const override { return height_map_name.c_str(); }
		void set_height_map(const std::string& name);
		void set_height_map(const char* name) override { set_height_map(std::string(name)); }
		const char* get_normal_map() const override { return normal_map_name.c_str(); }
		void set_normal_map(const std::string& name);
		void set_normal_map(const char* name) override { set_normal_map(std::string(name)); }
		const char* get_material_name() const override { return material_name.c_str(); }
		void set_material_name(const std::string& name);
		void set_material_name(const char* name) override { set_material_name(std::string(name)); }

		graphics::Image* get_height_texture() const override { return height_texture; }
		graphics::Image* get_normal_texture() const override { return normal_texture; }

		void draw(sRendererPrivate* s_renderer);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
