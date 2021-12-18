#pragma once

#include "blur.h"

namespace flame
{
	struct cBlurPrivate : cBlur
	{
		uint blur_radius = 0;

		cElementPrivate* element = nullptr;

		uint get_blur_radius() const override { return blur_radius; }
		void set_blur_radius(uint s) override;

		void draw(sRenderer* s_renderer);
	};
}
