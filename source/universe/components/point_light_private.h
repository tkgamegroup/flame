#pragma once

#include "point_light.h"

namespace flame
{
	struct cPointLightPrivate : cPointLight
	{
		bool dirty = true;

		~cPointLightPrivate();
		void on_init() override;
		
		void set_color(const vec4& color) override;
		void set_range(float range) override;
		void set_cast_shadow(bool cast_shadow) override;

		void on_active() override;
		void on_inactive() override;
	};
}
