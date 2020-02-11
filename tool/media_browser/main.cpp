#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>
#include <flame/utils/app.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
}app;

int main(int argc, char** args)
{
	app.create("Media Browser", Vec2u(1280, 720), WindowFrame | WindowResizable);

	ui::set_current_entity(app.root);
	ui::c_layout();
	ui::set_current_root(app.root);
	ui::push_parent(app.root);
		ui::e_begin_layout(LayoutHorizontal, 0.f, false, false);
		ui::c_aligner(SizeFitParent, SizeFitParent);
		ui::e_end_layout();
	ui::pop_parent();

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
