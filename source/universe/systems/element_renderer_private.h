#pragma once

#include <flame/universe/systems/element_renderer.h>

namespace flame
{
	struct EntityPrivate;

	struct sElementRendererPrivate : sElementRenderer
	{
		graphics::Canvas* _canvas = nullptr;

		bool _dirty = false;

		void _do_render(EntityPrivate* e);
		void _on_added();
		void _update();

		static sElementRendererPrivate* _create();

		void release() override { delete this; }

		void on_added() override { _on_added(); }

		void update() override { _update(); }
	};
}
