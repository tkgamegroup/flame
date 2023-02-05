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
		void set_tone_mapping_enable(bool v) override;
		void set_post_processing_enable(bool v) override;
		void set_ssr_enable(bool v) override;
		void set_ssr_thickness(float v) override;
		void set_ssr_step(float v) override;
		void set_ssr_max_steps(uint v) override;
		void set_ssr_binary_search_steps(uint v) override;
		void set_post_shading_code_file(const std::filesystem::path& path) override;

		void update_sky();
		void on_active() override;
		void on_inactive() override;
	};
}
