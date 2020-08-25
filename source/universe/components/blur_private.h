#pragma once

#include <flame/universe/components/blur.h>
#include "element_private.h"

namespace flame
{
	struct cBlurPrivate : cBlur // R ~ on_*
	{
		uint radius = 1.f;

		cElementPrivate* element = nullptr; // R ref

		uint get_radius() const override { return radius; }
		void set_radius(uint s) override;

		void draw_underlayer(graphics::Canvas* canvas); // R
	};
}
