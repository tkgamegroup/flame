#pragma once

#include <flame/universe/components/camera.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;

	struct cCameraPrivate : cCamera // R ~ on_*
	{
		float fovy = 60.f;
		float near = 0.1f;
		float far = 1.f;

		bool project_dirty = true;
		bool view_dirty = true;
		Mat4f project_matrix;
		Mat4f view_matrix;
		Mat4f vp_matrix; // P * V

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		void update_matrix();
	};
}