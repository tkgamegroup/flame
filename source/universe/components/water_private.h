#pragma once

#include "water.h"
#include "node_private.h"

namespace flame
{
	struct cWaterPrivate : cWater, NodeDrawer
	{
		vec2 extent = vec2(5.f);

		std::string material_name;

		ShadingFlags shading_flags = ShadingMaterial;

		cNodePrivate* node = nullptr;
		sRendererPrivate* s_renderer = nullptr;

		graphics::Material* material = nullptr;
		int material_id = -1;

		vec2 get_extent() const override { return extent; }
		void set_extent(const vec2& ext) override;

		const char* get_material_name() const override { return material_name.c_str(); }
		void set_material_name(std::string_view name);
		void set_material_name(const char* name) override { set_material_name(std::string(name)); }

		void draw(sRendererPtr s_renderer, bool, bool) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
