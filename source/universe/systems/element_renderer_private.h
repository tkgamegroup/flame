#pragma once

#include <flame/universe/systems/element_renderer.h>

namespace flame
{
	struct EntityPrivate;

	struct sElementRendererPrivate : sElementRenderer
	{
		graphics::Canvas* canvas = nullptr;

		bool dirty = false;

		void _do_render(EntityPrivate* e);
		void _update();

		void release() override { delete this; }

		void update() override { _update(); }
	};
}
