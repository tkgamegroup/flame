#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cRendererSettings : Component
	{
		/// Reflect
		std::filesystem::path sky_map_name; // will also load belonging _irr and _rad map
		/// Reflect
		virtual void set_sky_map_name(const std::filesystem::path& name) = 0;
		/// Reflect
		float sky_intensity = 1.f;
		/// Reflect
		virtual void set_sky_intensity(float v) = 0;
		/// Reflect
		vec3 fog_color = vec3(1.f);
		/// Reflect
		virtual void set_fog_color(const vec3& color) = 0;
		/// Reflect
		float shadow_distance = 100.f;
		/// Reflect
		virtual void set_shadow_distance(float d) = 0;
		/// Reflect
		uint csm_levels = 2;
		/// Reflect
		virtual void set_csm_levels(uint lv) = 0;
		/// Reflect
		float esm_factor = 7.f;
		/// Reflect
		virtual void set_esm_factor(float f) = 0;
		/// Reflect
		std::filesystem::path post_shading_code_file;
		/// Reflect
		virtual void set_post_shading_code_file(const std::filesystem::path& path) = 0;

		graphics::ImagePtr sky_map = nullptr;
		graphics::ImagePtr sky_irr_map = nullptr;
		graphics::ImagePtr sky_rad_map = nullptr;

		struct Create
		{
			virtual cRendererSettingsPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}