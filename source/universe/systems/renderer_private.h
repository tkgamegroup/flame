#pragma once

#include <flame/universe/systems/renderer.h>

namespace flame
{
	struct Window;

	namespace graphics
	{
		struct Device;
		struct Renderpass;
		struct Framebuffer;
		struct Pipeline;
		struct Swapchain;
	}

	struct EntityPrivate;
	struct cElementPrivate;
	struct cCameraPrivate;

	struct sRendererPrivate : sRenderer
	{
		bool hdr;

		Window* window;

		graphics::Device* device;
		graphics::Swapchain* swapchain;
		std::vector<FlmPtr<graphics::Framebuffer>> fb_targets;

		bool wireframe = false;
		graphics::Canvas* canvas = nullptr;
		cCameraPrivate* camera = nullptr;

		cElementPrivate* last_element;
		bool last_element_changed;

		bool always_update = false;
		bool dirty = true;

		sRendererPrivate(sRendererParms* parms);

		void set_targets();

		void render(EntityPrivate* e, bool element_culled, bool node_culled);

		void set_shade_wireframe(bool v) override { wireframe = v; }
		void set_always_update(bool a) override { always_update = a; }

		graphics::Canvas* get_canvas() const override { return canvas; }

		cCamera* get_camera() const override { return (cCamera*)camera; }
		void set_camera(cCamera* c) override { camera = (cCameraPrivate*)c; }

		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		void on_added() override;

		void update() override;
	};
}
