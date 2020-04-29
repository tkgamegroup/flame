#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>
#include <flame/utils/app.h>

using namespace flame;
using namespace graphics;

struct MainWindow : App::Window
{
	UI ui;

	MainWindow();
};

MainWindow* main_window;

struct MyApp : App
{
}app;

MainWindow::MainWindow() :
	App::Window(&app, true, true, "Media Browser", Vec2u(1280, 720), WindowFrame | WindowResizable)
{
	main_window = this;

	setup_as_main_window();

	ui.init(world);

	ui.parents.push(main_window->root);
		ui.e_begin_splitter(SplitterHorizontal);
		ui.e_element()->get_component(cElement)->color = Vec4c(255, 0, 0, 255);
		ui.c_aligner(AlignMinMax, AlignMinMax);
		ui.e_element()->get_component(cElement)->color = Vec4c(0, 255, 0, 255);
		ui.c_aligner(AlignMinMax, AlignMinMax);
		ui.e_end_splitter();
	ui.parents.pop();
}

int main(int argc, char** args)
{
	app.create();

	new MainWindow;

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
