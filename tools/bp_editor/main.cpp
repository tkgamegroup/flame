#include "app.h"

#include <flame/universe/utils/ui_impl.h>

void MyApp::create()
{
	App::create();
}

MyApp app;

int main(int argc, char **args)
{
	app.create(argc > 1 ? args[1] : "");

	new MainWindow;

	looper().add_event([](Capture& c) {
		if (app.auto_update)
			app.update();
		c._current = INVALID_POINTER;
	}, Capture(), 0.f);

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
