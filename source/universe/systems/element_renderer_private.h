#pragma once

#include <flame/universe/systems/element_renderer.h>

namespace flame
{
	struct EntityPrivate;

	struct sElementRendererPrivate : sElementRenderer
	{
		graphics::Canvas* canvas = nullptr;

		bool always_update = false;
		bool dirty = true;

		void set_always_update(bool a) override { always_update = a; }
		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		void on_added() override;

		void update() override;

		void do_render(EntityPrivate* e);
	};
}
