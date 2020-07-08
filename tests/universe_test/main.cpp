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
FontAtlas* font_atlas;

World* world;
sElementRenderer* ser;

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
	{
		Font* fonts[] = {
			Font::create(L"c:/windows/fonts/consola.ttf")
		};
		font_atlas = FontAtlas::create(d, size(fonts), fonts);
	}
	canvas->add_font(font_atlas);
	on_resize();

	set_allocator(
	[](Capture&, uint size) {
		return (void*)(new char[size]);
	},
	[](Capture&, void* p) {
		delete p;
	}, Capture());

	world = World::create();
	world->register_object(canvas, "Canvas");

	ser = sElementRenderer::create();
	world->add_system(ser);

	auto root = world->get_root();
	root->get_component( S<ch("abc")>::v );
	auto prefab_path = std::filesystem::path(getenv("FLAME_PATH")) / "art";
	root->load((prefab_path / "three.prefab").c_str());

	//add_file_watcher(prefab_path.c_str(), [](Capture& c, FileChangeType, const wchar_t* filename) {
	//	auto path = std::filesystem::path(filename);
	//	if (path.filename() == L"one.prefab")
	//	{
	//		auto root = c.thiz<Entity>();
	//		root->remove_all_components();
	//		root->remove_all_children();
	//		root->load(filename);
	//	}
	//}, Capture().set_thiz(root), false, false);

	//w->add_mouse_listener([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
	//	if (is_mouse_move(action, key))
	//	{
	//		auto e = (cElement*)c.thiz<Entity>()->get_component(cElement::type_hash);
	//		auto a = e->get_p00();
	//		auto b = e->get_p10();
	//		auto c = e->get_p11();
	//		auto d = e->get_p01();
	//		Vec2f points[] = { a, b, c, d };
	//		if (convex_contains<float>(Vec2f(pos), points))
	//			e->set_fill_color(Vec3c(255, 0, 0));
	//		else
	//			e->set_fill_color(Vec3c(255, 255, 255));
	//	}
	//}, Capture().set_thiz(root));

	get_looper()->loop([](Capture&, float) {
		if (!cbs.empty())
		{
			sc->acquire_image();

			auto img_idx = sc->get_image_index();
			auto cb = cbs[img_idx];

			canvas->prepare();

			ser->mark_dirty();
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
	}, Capture());

	return 0;
}
