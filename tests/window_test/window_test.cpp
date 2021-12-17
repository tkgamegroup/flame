#include <flame/foundation/application.h>

Application app;

int main(int argc, char** args)
{
	app.create("Window Test");
	app.main_window->add_mouse_left_down_listener([](const ivec2& pos) {
		app.main_window->close();
	});

	app.run();

	return 0;
}
