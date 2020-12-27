#pragma once

#include "../entity_private.h"
#include <flame/universe/components/post_effect.h>

namespace flame
{
	struct cElementPrivate;

	struct cPostEffectPrivate : cPostEffect
	{
		uint blur_radius = 0;
		bool enable_bloom = false;

		cElementPrivate* element = nullptr;

		uint get_blur_radius() const override { return blur_radius; }
		void set_blur_radius(uint s) override;

		bool get_enable_bloom() const override { return enable_bloom; }
		void set_enable_bloom(bool v) override;

		void draw(graphics::Canvas* canvas);
	};
}
