#pragma once

#include "environment.h"

namespace flame
{
	struct cEnvironmentPrivate : cEnvironment
	{
		~cEnvironmentPrivate();
		void on_init() override;

		void set_sky_map_name(const std::filesystem::path& name) override;

		void draw(sRendererPtr renderer);
	};
}
