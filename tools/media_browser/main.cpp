#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App g_app;

auto test_prefab = L"tree_test";

Entity* root;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, true, true, "Universe Test", Vec2u(600, 400), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(Vec4c(255));
	root = w->root;

	auto e = Entity::create();
	e->load(test_prefab);
	root->add_child(e);

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = INVALID_POINTER;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
