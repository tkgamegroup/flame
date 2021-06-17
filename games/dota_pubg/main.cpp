#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App g_app;

int main(int argc, char** args)
{
	g_app.create();

	auto w = new GraphicsWindow(&g_app, L"Dota Pubg", uvec2(800, 600), WindowFrame | WindowResizable, true);

	{
		auto e = Entity::create();
		e->load(L"main");
		w->root->add_child(e);
	}

	looper().add_event([](Capture& c) {
		printf("fps: %d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
