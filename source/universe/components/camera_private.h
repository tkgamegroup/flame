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

		cNodePrivate* node = nullptr;
		sRendererPrivate* renderer = nullptr;

		void apply_current();

		bool get_current() const override { return current; }
		void set_current(bool v) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
