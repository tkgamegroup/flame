#pragma once

#include <flame/universe/systems/renderer.h>

namespace flame
{
	struct EntityPrivate;
	struct cCameraPrivate;

	struct sRendererPrivate : sRenderer
	{
		graphics::Canvas* canvas = nullptr;
		cCameraPrivate* camera = nullptr;

		bool always_update = false;
		bool dirty = true;

		void set_always_update(bool a) override { always_update = a; }

		cCamera* get_camera() const override { return (cCamera*)camera; }
		void set_camera(cCamera* c) override { camera = (cCameraPrivate*)c; }

		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		void on_added() override;

		void update() override;

		void render(EntityPrivate* e, bool element_culled, bool node_culled);
	};
}