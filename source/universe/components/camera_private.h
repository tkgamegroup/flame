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
		sRendererPrivate* renderer = nullptr;

		void apply_current();

		void on_gain_renderer();
		void on_lost_renderer();

		void draw(graphics::Canvas* canvas);

		bool get_current() const override { return current; }
		void set_current(bool v) override;
	};
}
