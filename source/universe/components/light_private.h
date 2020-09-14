#pragma once

#include <flame/universe/components/light.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;
	struct cCamera;

	struct cLightPrivate : cLight // R ~ on_*
	{
		graphics::LightType type = graphics::LightPoint;
		Vec3f color = Vec3f(1.f);
		bool cast_shadow = false;

		cNodePrivate* node = nullptr; // R ref

		graphics::LightType get_type() const override { return type; }
		void set_type(graphics::LightType t) override;

		Vec3f get_color() const override { return color; }
		void set_color(const Vec3f& c) override;

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void draw(graphics::Canvas* canvas); // R
	};
}
