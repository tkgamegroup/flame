#pragma once

#include "renderer_settings.h"

namespace flame
{
	struct cRendererSettingsPrivate : cRendererSettings
	{
		void set_sky_map_name(const std::filesystem::path& name) override;
		void set_sky_intensity(float v) override;
		void set_fog_color(const vec3& color) override;
		void set_shadow_distance(float d) override;
		void set_csm_levels(uint lv) override;
		void set_esm_factor(float f) override;

		void update_sky();
		void on_active() override;
		void on_inactive() override;
	};
}
