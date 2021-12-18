#pragma once

#include "light.h"
#include "node_private.h"

namespace flame
{
	struct cLightPrivate : cLight, NodeDrawer
	{
		LightType type = LightPoint;
		vec3 color = vec3(1.f);
		bool cast_shadow = false;

		cNodePrivate* node = nullptr;
		int id = -1;
		sRendererPrivate* s_renderer = nullptr;

		LightType get_type() const override { return type; }
		void set_type(LightType t) override;

		vec3 get_color() const override { return color; }
		void set_color(const vec3& c) override;

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		mat4 get_shadow_mat(uint idx) const override;

		void draw(sRendererPtr s_renderer, bool, bool) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
