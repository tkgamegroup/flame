#pragma once

#include "../entity_private.h"
#include <flame/universe/components/light.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;
	struct cCamera;

	struct cLightPrivate : cLight
	{
		graphics::LightType type = graphics::LightPoint;
		vec3 color = vec3(1.f);
		bool cast_shadow = false;

		cNodePrivate* node = nullptr;

		graphics::LightType get_type() const override { return type; }
		void set_type(graphics::LightType t) override;

		vec3 get_color() const override { return color; }
		void set_color(const vec3& c) override;

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void draw(graphics::Canvas* canvas);
	};
}
