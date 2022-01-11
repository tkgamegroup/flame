#include <flame/foundation/application.h>

Application app;

int main(int argc, char** args)
{
	app.create("Window Test");
	app.main_window->mouse_listeners.add([](MouseButton btn, bool down) {
		if (btn == Mouse_Left && down)
			app.main_window->close();
	});

	app.run();

	return 0;
}
