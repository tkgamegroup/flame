#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cRendererSettings : Component
	{
		// Reflect
		std::filesystem::path sky_map_name; // will also load belonging _irr and _rad map
		// Reflect
		virtual void set_sky_map_name(const std::filesystem::path& name) = 0;
		// Reflect
		float sky_intensity = 1.f;
		// Reflect
		virtual void set_sky_intensity(float v) = 0;
		// Reflect
		vec3 fog_color = vec3(1.f);
		// Reflect
		virtual void set_fog_color(const vec3& color) = 0;
		// Reflect
		float shadow_distance = 100.f;
		// Reflect
		virtual void set_shadow_distance(float d) = 0;
		// Reflect
		uint csm_levels = 2;
		// Reflect
		virtual void set_csm_levels(uint lv) = 0;
		// Reflect
		float esm_factor = 100.f;
		// Reflect
		virtual void set_esm_factor(float f) = 0;
		// Reflect
		bool post_processing_enable = true;
		// Reflect
		virtual void set_post_processing_enable(bool v) = 0;
		// Reflect
		bool ssao_enable = true;
		// Reflect
		virtual void set_ssao_enable(bool v) = 0;
		// Reflect
		float ssao_radius = 0.5f;
		// Reflect
		virtual void set_ssao_radius(float v) = 0;
		// Reflect
		float ssao_bias = 0.025f;
		// Reflect
		virtual void set_ssao_bias(float v) = 0;
		// Reflect
		float white_point = 4.f;
		// Reflect
		virtual void set_white_point(float v) = 0;
		// Reflect
		bool bloom_enable = true;
		// Reflect
		virtual void set_bloom_enable(bool v) = 0;
		// Reflect
		bool ssr_enable = true;
		// Reflect
		virtual void set_ssr_enable(bool v) = 0;
		// Reflect
		float ssr_thickness = 0.5f;
		// Reflect
		virtual void set_ssr_thickness(float v) = 0;
		// Reflect
		float ssr_max_distance = 8.f;
		// Reflect
		virtual void set_ssr_max_distance(float v) = 0;
		// Reflect
		uint ssr_max_steps = 64;
		// Reflect
		virtual void set_ssr_max_steps(uint v) = 0;
		// Reflect
		uint ssr_binary_search_steps = 5;
		// Reflect
		virtual void set_ssr_binary_search_steps(uint v) = 0;
		// Reflect
		bool tone_mapping_enable = true;
		// Reflect
		virtual void set_tone_mapping_enable(bool v) = 0;
		// Reflect
		float gamma = 1.5f;
		// Reflect
		virtual void set_gamma(float v) = 0;
		// Reflect
		std::filesystem::path post_shading_code_file;
		// Reflect
		virtual void set_post_shading_code_file(const std::filesystem::path& path) = 0;

		graphics::ImagePtr sky_map = nullptr;
		graphics::ImagePtr sky_irr_map = nullptr;
		graphics::ImagePtr sky_rad_map = nullptr;

		struct Create
		{
			virtual cRendererSettingsPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
