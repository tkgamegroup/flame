#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App g_app;

auto test_prefab = L"tree_test";

Entity* root;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, true, true, "Universe Test", uvec2(600, 400), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(cvec4(255));
	root = w->root;

	auto e = Entity::create();
	e->load(test_prefab);
	e->save(L"d:/1.prefab");
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
