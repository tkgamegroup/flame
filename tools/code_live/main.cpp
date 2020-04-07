#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
	graphics::FontAtlas* font_atlas_edit;
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.create("UI Test", Vec2u(600, 400), WindowFrame | WindowResizable, true, engine_path);

	const wchar_t* fonts[] = {
		L"c:/windows/fonts/consola.ttf"
	};
	app.font_atlas_edit = graphics::FontAtlas::create(app.graphics_device, 1, fonts);
	app.canvas->add_font(app.font_atlas_edit);

	utils::push_parent(app.root);
		utils::e_begin_splitter(SplitterHorizontal);
			utils::e_begin_layout(LayoutVertical, 4.f);
			utils::c_aligner(SizeFitParent, SizeFitParent);
			utils::e_edit(100.f)->get_component(cText)->font_atlas = app.font_atlas_edit;
			utils::c_aligner(SizeFitParent, SizeFitParent);
			utils::e_button(L"Run");
			utils::e_end_layout();

			utils::e_element();
			utils::c_aligner(SizeFitParent, SizeFitParent);
		utils::e_end_splitter();
	utils::pop_parent();

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
