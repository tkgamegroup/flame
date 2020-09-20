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
		float fovy = 45.f;
		float near = 1.f;
		float far = 100.f;

		cNodePrivate* node = nullptr; // R ref
	};
}
