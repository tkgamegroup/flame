#pragma once

#include <flame/universe/components/sky.h>

namespace flame
{
	namespace graphics
	{
		struct Image;
		struct Canvas;
	}

	struct cNodePrivate;

	struct cSkyPrivate : cSky // R ~ on_*
	{
		std::string box_texture_name;
		std::string irr_texture_name;
		std::string rad_texture_name;
		graphics::Image* box_texture = nullptr;
		graphics::Image* irr_texture = nullptr;
		graphics::Image* rad_texture = nullptr;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		const char* get_box_texture() const override { return box_texture_name.c_str(); }
		void set_box_texture(const char* name) override;
		const char* get_irr_texture() const override { return irr_texture_name.c_str(); }
		void set_irr_texture(const char* name) override;
		const char* get_rad_texture() const override { return rad_texture_name.c_str(); }
		void set_rad_texture(const char* name) override;

		void on_gain_canvas();
	};
}
