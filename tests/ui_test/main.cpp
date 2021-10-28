#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App app;

int main(int argc, char** args)
{
	app.create();
	new GraphicsWindow(&app, L"UI Test", uvec2(1280, 720), NativeWindowFrame | NativeWindowResizable);
	{
		auto e = Entity::create();
		e->load(L"ui_test");
		app.main_window->root->add_child(e);
	}
	app.run();

	return 0;
}
