#pragma once

#include <flame/universe/components/sky.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;

	struct cSkyPrivate : cSky // R ~ on_*
	{
		std::string texture_name;
		int texture_id = -1;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		const char* get_texture_name() const override { return texture_name.c_str(); }
		void set_texture_name(const char* name) override;

		void on_gain_canvas();

		void draw(graphics::Canvas* canvas); // R
	};
}
