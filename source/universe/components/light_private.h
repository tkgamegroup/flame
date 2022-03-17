#pragma once

#include "light.h"

namespace flame
{
	struct cLightPrivate : cLight
	{
		~cLightPrivate();
		void on_init() override;

		void set_type(LightType type) override;
		void set_color(const vec4& color) override;
		void set_range(float range) override;
		void set_cast_shadow(bool cast_shadow) override;

		void draw(sRendererPtr renderer);

		void on_active() override;
		void on_inactive() override;
	};
}
