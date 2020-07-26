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
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

Window* w;
Device* d;
Queue* q;
Swapchain* sc;
int img_idx = -1;
Fence* fence;
std::vector<CommandBuffer*> cbs;
Semaphore* render_finished;

Canvas* canvas;
FontAtlas* font_atlas;

World* world;
sElementRenderer* renderer = nullptr;

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
auto test_prefab = L"checkbox.prefab";

int main(int argc, char** args)
{
	w = Window::create("Universe Test", Vec2u(600, 400), WindowFrame | WindowResizable);
	d = Device::create(true);
	q = d->get_queue(QueueGraphics);
	render_finished = Semaphore::create(d);
	sc = Swapchain::create(d, w, ImageUsageSampled);
	fence = Fence::create(d);
	w->add_resize_listener([](Capture&, const Vec2u&) {
		on_resize();
	}, Capture());

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
	},
	[](void* p, uint size) {
		auto& str = *(std::wstring*)p;
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
	struct sPrepareCanvas : System
	{
		sPrepareCanvas() :
			System("sPrepareCanvas", ch("sEventDispatcher"))
		{
		}

		void update() override
		{
			if (!renderer->is_dirty())
				return;
			if (img_idx < 0)
			{
				if (!cbs.empty())
				{
					sc->acquire_image();
					img_idx = sc->get_image_index();
				}
				canvas->prepare();
			}
		}
	};
	world->add_system(new sPrepareCanvas);
	renderer = sElementRenderer::create();
	world->add_system(renderer);

	auto e = Entity::create();
	e->load((res_path / test_prefab).c_str());
	world->get_root()->add_child(e);

	//add_file_watcher(res_path.c_str(), [](Capture& c, FileChangeType, const wchar_t* filename) {
	//	auto path = std::filesystem::path(filename);
	//	if (path.filename() == test_prefab)
	//	{
	//		auto e = Entity::create();
	//		e->load(filename);
	//		auto root = c.thiz<Entity>();
	//		root->remove_all_children();
	//		root->add_child(e);
	//	}
	//}, Capture().set_thiz(root), false, false);

	//w->add_key_listener([](Capture& c, KeyStateFlags action, int value) {
	//	if (is_key_down(action) && value == Key_Right)
	//	{
	//		auto e = (cElement*)c.thiz<Entity>()->get_component(cElement::type_hash);
	//		e->set_x(e->get_x() + 1);
	//	}
	//}, Capture().set_thiz(root));

	add_fps_listener([](Capture&, uint fps) {
		printf("%d\n", fps);
	}, Capture());

	get_looper()->loop([](Capture&, float) {
		{
			auto t = (0.02 - get_looper()->get_delta_time());
			if (t > 0.f)
				std::this_thread::sleep_for(std::chrono::milliseconds(uint(t * 1000)));
		}

		if (!cbs.empty())
		{
			world->update();

			fence->wait();

			if (img_idx >= 0)
			{
				auto cb = cbs[img_idx];

				canvas->record(cb, img_idx);

				q->submit(1, &cb, sc->get_image_avalible(), render_finished, fence);
				q->present(sc, render_finished);
				img_idx = -1;
			}
		}
	}, Capture());

	return 0;
}
