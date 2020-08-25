#pragma once

#include <flame/universe/components/object.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;
	struct cCameraPrivate;

	struct cObjectPrivate : cObject // R ~ on_*
	{
		cNodePrivate* node = nullptr; // R ref

		void draw(graphics::Canvas* canvas, cCamera* camera); // R
	};
}
