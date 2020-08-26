#include <flame/universe/app.h>
#include <flame/universe/components/camera.h>

using namespace flame;
using namespace graphics;

App g_app;

auto test_prefab = L"text";

Entity* root;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, true, true, "Universe Test", Vec2u(600, 400), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	root = w->root;

	auto e = Entity::create();
	e->load(test_prefab);
	{
		auto n = e->find_child("camera");
		if (n)
			w->s_renderer->set_camera(n->get_component_t<cCamera>());
	}
	//e->save(L"d:/1.prefab");
	root->add_child(e);

	g_app.script_instance->add_object(e, "entity", "flame__Entity");
	g_app.script_instance->excute(L"d:/1.lua");

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
		c._current = INVALID_POINTER;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
