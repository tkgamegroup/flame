#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/utils/app.h>

#include <flame/universe/utils/ui_impl.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
}app;

int main(int argc, char** args)
{
	app.create();
	auto main_window = new App::Window(&app, true, true, "Media Browser", Vec2u(1280, 720), WindowFrame | WindowResizable);

	utils::push_parent(main_window->root);
		utils::e_begin_splitter(SplitterHorizontal);
		utils::e_element()->get_component(cElement)->color = Vec4c(255, 0, 0, 255);
		utils::c_aligner(AlignMinMax, AlignMinMax);
		utils::e_element()->get_component(cElement)->color = Vec4c(0, 255, 0, 255);
		utils::c_aligner(AlignMinMax, AlignMinMax);
		utils::e_end_splitter();
	utils::pop_parent();

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
