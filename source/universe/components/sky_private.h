#pragma once

#include "sky.h"

namespace flame
{
	struct cSkyPrivate : cSky
	{
		std::filesystem::path box_texture_path;
		std::filesystem::path irr_texture_path;
		std::filesystem::path rad_texture_path;
		std::filesystem::path lut_texture_path;

		sRendererPrivate* s_renderer = nullptr;

		graphics::Image* box_texture = nullptr;
		graphics::ImageView* box_texture_view = nullptr;
		graphics::Image* irr_texture = nullptr;
		graphics::ImageView* irr_texture_view = nullptr;
		graphics::Image* rad_texture = nullptr;
		graphics::ImageView* rad_texture_view = nullptr;
		graphics::Image* lut_texture = nullptr;
		graphics::ImageView* lut_texture_view = nullptr;

		vec3 fog_color = vec3(0.f);
		float intensity = 0.2f;

		const wchar_t* get_box_texture_path() const override { return box_texture_path.c_str(); }
		void set_box_texture_path(const std::filesystem::path& path);
		void set_box_texture_path(const wchar_t* path) override { set_box_texture_path(std::filesystem::path(path)); }
		const wchar_t* get_irr_texture_path() const override { return irr_texture_path.c_str(); }
		void set_irr_texture_path(const std::filesystem::path& path);
		void set_irr_texture_path(const wchar_t* path) override { set_irr_texture_path(std::filesystem::path(path)); }
		const wchar_t* get_rad_texture_path() const override { return rad_texture_path.c_str(); }
		void set_rad_texture_path(const std::filesystem::path& path);
		void set_rad_texture_path(const wchar_t* path) override { set_rad_texture_path(std::filesystem::path(path)); }
		const wchar_t* get_lut_texture_path() const override { return lut_texture_path.c_str(); }
		void set_lut_texture_path(const std::filesystem::path& path);
		void set_lut_texture_path(const wchar_t* path) override { set_lut_texture_path(std::filesystem::path(path)); }

		float get_intensity() const override { return intensity; }
		void set_intensity(float i) override;

		void on_entered_world() override;
		void on_left_world() override;
	};
}
