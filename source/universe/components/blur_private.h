#pragma once

#include <flame/universe/components/blur.h>
#include "element_private.h"

namespace flame
{
	struct cBlurPrivate : cBlur, cElement::Drawer // R ~ on_*
	{
		float sigma = 1.f;

		cElementPrivate* element = nullptr; // R ref

		float get_sigma() const override { return sigma; }
		void set_sigma(float s) override;

		void on_gain_element();
		void on_lost_element();

		void draw(graphics::Canvas* canvas) override;
	};
}
