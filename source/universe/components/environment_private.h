#pragma once

#include "environment.h"

namespace flame
{
	struct cEnvironmentPrivate : cEnvironment
	{
		void set_sky_map_name(const std::filesystem::path& name) override;
		void set_sky_intensity(float v) override;
		void set_fog_color(const vec3& color) override;

		void update_sky();
		void on_active() override;
		void on_inactive() override;
	};
}
