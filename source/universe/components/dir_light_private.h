#pragma once

#include "dir_light.h"

namespace flame
{
	struct cDirLightPrivate : cDirLight
	{
		~cDirLightPrivate();
		void on_init() override;

		void set_color(const vec4& color) override;
		void set_cast_shadow(bool cast_shadow) override;

		void on_active() override;
		void on_inactive() override;
	};
}
