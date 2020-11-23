#pragma once

#include <flame/universe/components/camera.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;
	struct sRendererPrivate;

	struct cCameraPrivate : cCamera // R ~ on_*
	{
		float fovy = 40.f;
		float near = 1.f;
		float far = 1000.f;

		bool current = false;

		cNodePrivate* node = nullptr; // R ref
		sRendererPrivate* renderer = nullptr; // R ref

		void apply_current();

		void on_gain_renderer();
		void on_lost_renderer();

		void draw(graphics::Canvas* canvas); // R

		bool get_current() const override { return current; }
		void set_current(bool v) override;
	};
}
