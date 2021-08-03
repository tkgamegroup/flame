#pragma once

#include "../entity_private.h"
#include "camera.h"

namespace flame
{
	struct cCameraPrivate : cCamera
	{
		float fovy = 45.f;
		float near = 1.f;
		float far = 1000.f;
		uvec2 screen_size = uvec2(1);
		float aspect = 1.f;

		bool current = false;

		uint view_mark = 0;
		mat4 view;
		mat4 view_inv;
		bool proj_dirty = true;
		mat4 proj;
		mat4 proj_inv;

		cNodePrivate* node = nullptr;
		sRendererPrivate* s_renderer = nullptr;

		void apply_current();

		float get_fovy() const override { return fovy; }
		void set_fovy(float v) override;
		float get_near() const override { return near; }
		void set_near(float v) override;
		float get_far() const override { return far; }
		void set_far(float v) override;
		uvec2 get_screen_size() const override { return screen_size; }
		void set_screen_size(const uvec2& v) override;

		void update_view();
		void update_proj();

		bool get_current() const override { return current; }
		void set_current(bool v) override;

		void get_points(vec3* dst, float n = -1.f, float f = -1.f) override;
		Frustum get_frustum(float n = -1.f, float f = -1.f) override;

		vec3 screen_to_world(const uvec2& pos) override;
		ivec2 world_to_screen(const vec3& pos, const ivec4& border) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
