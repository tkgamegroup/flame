#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct Window;

	namespace graphics
	{
		struct Canvas;
	}

	struct cWidget$;

	struct cUI$ : Component
	{
		cWidget$* hovering;
		cWidget$* focusing;

		FLAME_UNIVERSE_EXPORTS virtual ~cUI$() override;

		FLAME_UNIVERSE_EXPORTS virtual const char* type_name() const override;
		FLAME_UNIVERSE_EXPORTS virtual uint type_hash() const override;

		FLAME_UNIVERSE_EXPORTS virtual void update(float delta_time) override;

		FLAME_UNIVERSE_EXPORTS graphics::Canvas* canvas() const;
		FLAME_UNIVERSE_EXPORTS void setup(graphics::Canvas* canvas, Window* window = nullptr);

		FLAME_UNIVERSE_EXPORTS static cUI$* create$(void* data);
	};
}
