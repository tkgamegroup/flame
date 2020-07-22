#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/command.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/world.h>
#include <flame/universe/entity.h>
#include <flame/universe/res_map.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/systems/type_setting.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/element_renderer.h>

using namespace flame;
using namespace graphics;

Window* w;
Device* d;
Swapchain* sc;
Fence* fence;
std::vector<CommandBuffer*> cbs;
Semaphore* render_finished;

Canvas* canvas;
FontAtlas* font_atlas;

World* world;
sElementRenderer* ser;

void on_resize()
{
	std::vector<ImageView*> vs(sc->get_images_count());
	for (auto i = 0; i < vs.size(); i++)
		vs[i] = sc->get_image(i)->get_default_view();

	canvas->set_target(vs.size(), vs.data());

	for (auto cb : cbs)
		cb->release();
	cbs.resize(vs.size());
	for (auto i = 0; i < cbs.size(); i++)
		cbs[i] = CommandBuffer::create(d->get_command_pool(QueueGraphics));
}

auto res_path = std::filesystem::path(getenv("FLAME_PATH")) / "art";
auto test_prefab = L"8.prefab";

int main(int argc, char** args)
{
	w = Window::create("Universe Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	d = Device::create(true);
	render_finished = Semaphore::create(d);
	sc = Swapchain::create(d, w, ImageUsageSampled);
	fence = Fence::create(d);

	canvas = Canvas::create(d);
	canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	{
		Font* fonts[] = {
			Font::create(L"c:/windows/fonts/consola.ttf")
		};
		font_atlas = FontAtlas::create(d, size(fonts), fonts);
		canvas->set_resource(-1, font_atlas->get_view(), d->get_sampler(FilterNearest), L"", nullptr, font_atlas);
	}

	on_resize();

	set_allocator(
	[](uint size) {
		return malloc(size);
	},
	[](void* p) {
		free(p);
	},
	[](void* p, uint size) {
		return realloc(p, size);
	},
	[](void* p, uint size) {
		auto& str = *(std::string*)p;
		str.resize(size);
		return str.data();
	});

	world = World::create();
	world->register_object(w, "Window");
	world->register_object(canvas, "Canvas");
	auto res = ResMap::create();
	res->load((res_path / L"res.ini").c_str());
	res->traversal([](Capture&, const char*, const wchar_t* _path) {
		auto path = std::filesystem::path(_path);
		canvas->set_resource(-1, Image::create(d, path.c_str())->get_default_view(), nullptr, path.c_str());
	}, Capture());
	world->register_object(res, "ResMap");

	world->add_system(sTypeSetting::create());
	world->add_system(sEventDispatcher::create());
	ser = sElementRenderer::create();
	world->add_system(ser);

	auto root = world->get_root();
	root->load((res_path / test_prefab).c_str());

	//add_file_watcher(res_path.c_str(), [](Capture& c, FileChangeType, const wchar_t* filename) {
	//	auto path = std::filesystem::path(filename);
	//	if (path.filename() == test_prefab)
	//	{
	//		auto root = c.thiz<Entity>();
	//		root->remove_all_components();
	//		root->remove_all_children();
	//		root->load(filename);
	//	}
	//}, Capture().set_thiz(root), false, false);

	//w->add_key_listener([](Capture& c, KeyStateFlags action, int value) {
	//	if (is_key_down(action) && value == Key_Right)
	//	{
	//		auto e = (cElement*)c.thiz<Entity>()->get_component(cElement::type_hash);
	//		e->set_x(e->get_x() + 1);
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
				auto q = d->get_queue(QueueGraphics);
				q->submit(1, &cb, sc->get_image_avalible(), render_finished, fence);
				q->present(sc, render_finished);
			}
		}
	}, Capture());

	return 0;
}
