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

		cNodePrivate* node = nullptr; // R ref

		graphics::LightType get_type() const override { return type; }
		void set_type(graphics::LightType t) override;

		Vec3f get_color() const override { return color; }
		void set_color(const Vec3f& c) override;

		void draw(graphics::Canvas* canvas, cCamera* camera); // R
	};
}
