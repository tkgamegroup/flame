#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App app;

int main(int argc, char** args)
{
	app.create();
	app.set_main_window(Window::create(nullptr, NativeWindow::create(L"UI Test", uvec2(1280, 720), WindowFrame | WindowResizable)));

	{
		auto e = Entity::create();
		e->load(L"ui_test");
		app.root->add_child(e);
	}

	run([](Capture& c, float) {
		app.update();
	}, Capture());

	return 0;
}
