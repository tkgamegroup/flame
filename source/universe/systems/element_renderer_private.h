#pragma once

#include <flame/universe/systems/element_renderer.h>

namespace flame
{
	struct EntityPrivate;

	struct sElementRendererPrivate : sElementRenderer
	{
		graphics::Canvas* canvas = nullptr;

		bool dirty = true;

		bool is_dirty() const override { return dirty; }
		void mark_dirty() override { dirty = true; }

		void on_added() override;

		void update() override;

		void do_render(EntityPrivate* e);
	};
}
