#pragma once

#include "../entity_private.h"
#include <flame/universe/components/camera.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;
	struct sRendererPrivate;

	struct cCameraPrivate : cCamera
	{
		float fovy = 40.f;
		float near = 1.f;
		float far = 1000.f;

		bool current = false;

		cNodePrivate* node = nullptr;
		void* drawer = nullptr;
		sRendererPrivate* renderer = nullptr;

		void apply_current();

		bool get_current() const override { return current; }
		void set_current(bool v) override;

		void draw(graphics::Canvas* canvas);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
