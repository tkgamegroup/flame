#pragma once

#include <flame/universe/systems/element_renderer.h>

namespace flame
{
	struct EntityPrivate;

	struct sElementRendererPrivate : sElementRenderer
	{
		graphics::Canvas* canvas = nullptr;

		bool dirty = false;

		void mark_dirty() override { dirty = true; }

		void on_added() override;

		void update() override;

		void do_render(EntityPrivate* e);

		static sElementRendererPrivate* create();
	};
}
