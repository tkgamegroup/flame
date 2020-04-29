#include "app.h"

#include <flame/universe/utils/ui_impl.h>

int main(int argc, char **args)
{
	app.create();

	new MainWindow;

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
