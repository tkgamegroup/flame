#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App g_app;

auto test_prefab = std::filesystem::path(L"scene_test");

int main(int argc, char** args)
{
	g_app.create();

	auto w = new GraphicsWindow(&g_app, L"Universe Test", uvec2(800, 600), WindowFrame | WindowResizable, true);

	//w->canvas->set_clear_color(cvec4(100, 100, 100, 255));

	{
		auto e = Entity::create();
		e->load(test_prefab.c_str());
		//e->save(L"d:/1.prefab");
		w->root->add_child(e);
	}

	looper().add_event([](Capture& c) {
		printf("fps: %d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
