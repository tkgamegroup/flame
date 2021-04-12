#pragma once

#include "sky.h"

namespace flame
{
	struct cSkyPrivate : cSky
	{
		std::string box_texture_name;
		std::string irr_texture_name;
		std::string rad_texture_name;
		std::string lut_texture_name;

		cNodePrivate* node = nullptr;
		sRendererPrivate* renderer = nullptr;

		graphics::Image* box_texture;
		graphics::Image* irr_texture;
		graphics::Image* rad_texture;
		graphics::Image* lut_texture;

		const char* get_box_texture() const override { return box_texture_name.c_str(); }
		void set_box_texture(const char* name) override;
		const char* get_irr_texture() const override { return irr_texture_name.c_str(); }
		void set_irr_texture(const char* name) override;
		const char* get_rad_texture() const override { return rad_texture_name.c_str(); }
		void set_rad_texture(const char* name) override;
		const char* get_lut_texture() const override { return lut_texture_name.c_str(); }
		void set_lut_texture(const char* name) override;

		void on_entered_world() override;
		void on_left_world() override;
	};
}
