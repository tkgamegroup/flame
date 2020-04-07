#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable, true, engine_path);

	utils::set_current_entity(app.root);
	utils::c_event_receiver();
	utils::c_layout();
	utils::push_font_atlas(app.font_atlas_pixel);

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
