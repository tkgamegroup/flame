#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/world.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/systems/element_renderer.h>

using namespace flame;
using namespace graphics;

Window* w;
Device* d;
Swapchain* sc;
Fence* fence;
std::vector<Commandbuffer*> cbs;
Semaphore* render_finished;

graphics::Canvas* canvas;

World* world;

void on_resize()
{
	std::vector<Imageview*> vs(sc->get_images_count());
	for (auto i = 0; i < vs.size(); i++)
		vs[i] = sc->get_image(i)->get_default_view();

	canvas->set_target(vs.size(), vs.data());

	for (auto cb : cbs)
		cb->release();
	cbs.resize(vs.size());
	for (auto i = 0; i < cbs.size(); i++)
		cbs[i] = Commandbuffer::create(d->get_graphics_commandpool());
}

int main(int argc, char** args)
{
	w = Window::create("Graphics Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	d = Device::create(true);
	render_finished = Semaphore::create(d);
	sc = Swapchain::create(d, w);
	fence = Fence::create(d);
	canvas = Canvas::create(d);
	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	on_resize();

	set_allocator(
	[](Capture&, uint size) {
		return (void*)(new char[size]);
	},
	[](Capture&, void* p) {
		delete p;
	}, Capture());

	world = World::create();
	world->register_object(canvas);

	world->add_system(sElementRenderer::create());

	auto root = world->get_root();
	auto ce = cElement::create();
	ce->set_x(100.f);
	ce->set_y(50.f);
	ce->set_width(200.f);
	ce->set_height(100.f);
	root->add_component(ce);

	if (!cbs.empty())
	{
		sc->acquire_image();

		auto img_idx = sc->get_image_index();
		auto cb = cbs[img_idx];

		canvas->prepare();

		world->update();

		canvas->record(cb, img_idx);

		fence->wait();

		if (!cbs.empty())
		{
			auto q = d->get_graphics_queue();
			q->submit(1, &cb, sc->get_image_avalible(), render_finished, fence);
			q->present(sc, render_finished);
		}
	}
	get_looper()->loop([](Capture&, float) {
	}, Capture());

	return 0;
}
