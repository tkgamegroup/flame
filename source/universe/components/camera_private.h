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

		bool current = false;

		uint view_mark = 0;
		mat4 view;
		mat4 view_inv;

		cNodePrivate* node = nullptr;
		sRendererPrivate* s_renderer = nullptr;

		void apply_current();

		bool get_current() const override { return current; }
		void set_current(bool v) override;

		void update_view();

		void get_points(float aspect, vec3* dst, float n = -1.f, float f = -1.f);
		void get_planes(float aspect, Plane* dst, float n = -1.f, float f = -1.f);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
