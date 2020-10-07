#include <flame/graphics/model.h>
#include <flame/universe/app.h>
#include <flame/universe/components/camera.h>

using namespace flame;
using namespace graphics;

App g_app;

auto test_prefab = std::filesystem::path(L"tests/model_test");
//auto model_path = std::filesystem::path(LR"(D:\island\Small_Tropical_Island\Small_Tropical_Island.fmod)");
auto model_path = std::filesystem::path(LR"(D:\illidan\illidan.fmod)");

Entity* root;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, "Universe Test", Vec2u(600, 400), WindowFrame | WindowResizable, true, true);
	w->canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	if (!model_path.empty())
	{
		w->canvas->set_resource(graphics::ResourceModel, -1, Model::create(model_path.c_str()), "mod");
		model_path.replace_extension(L".prefab");
		Entity::register_prefab(model_path.c_str(), "mod");
	}
	{
		auto t = Image::create(g_app.graphics_device, L"D:/terrain/height.png", false);
		w->canvas->set_resource(ResourceTexture, -1, t->get_view(0), "height_map");
	}
	{
		auto t = Image::create(g_app.graphics_device, L"D:/terrain/color.png", true);
		w->canvas->set_resource(ResourceTexture, -1, t->get_view(0), "color_map");
	}
	root = w->root;

	auto e = Entity::create();
	e->load(test_prefab.c_str());
	{
		auto n = e->find_child("camera");
		if (n)
			w->s_renderer->set_camera(n->get_component_t<cCamera>());
	}
	//e->save(L"d:/1.prefab");
	root->add_child(e);

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

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
