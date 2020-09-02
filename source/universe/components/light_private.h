#pragma once

#include <flame/universe/components/light.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;
	struct cCamera;

	struct cLightPrivate : cLight // R ~ on_*
	{
		cNodePrivate* node = nullptr; // R ref

		void draw(graphics::Canvas* canvas, cCamera* camera); // R
	};
}
