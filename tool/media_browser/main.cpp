#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>
#include <flame/utils/app.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.create("Media Browser", Vec2u(1280, 720), WindowFrame | WindowResizable, true, engine_path);

	utils::set_current_entity(app.root);
	utils::c_layout();
	utils::set_current_root(app.root);
	utils::push_parent(app.root);
		utils::e_begin_splitter(SplitterHorizontal);
		utils::e_element()->get_component(cElement)->color_ = Vec4c(255, 0, 0, 255);
		utils::c_aligner(SizeFitParent, SizeFitParent);
		utils::e_element()->get_component(cElement)->color_ = Vec4c(0, 255, 0, 255);
		utils::c_aligner(SizeFitParent, SizeFitParent);
		utils::e_end_splitter();
	utils::pop_parent();

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
