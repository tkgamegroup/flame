#pragma once

#include <flame/universe/components/blur.h>
#include "element_private.h"

namespace flame
{
	struct cBlurPrivate : cBlur, cElement::Drawer // R ~ on_*
	{
		uint radius = 1.f;

		cElementPrivate* element = nullptr; // R ref

		uint get_radius() const override { return radius; }
		void set_radius(uint s) override;

		void on_gain_element();
		void on_lost_element();

		void draw(graphics::Canvas* canvas) override;
	};
}
