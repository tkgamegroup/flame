#pragma once

#include "../entity_private.h"
#include <flame/universe/components/blur.h>

namespace flame
{
	struct cElementPrivate;

	struct cBlurPrivate : cBlur
	{
		uint radius = 1.f;

		cElementPrivate* element = nullptr;

		uint get_radius() const override { return radius; }
		void set_radius(uint s) override;

		void draw0(graphics::Canvas* canvas);
	};
}
