#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/image.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/application.h>

using namespace flame;
using namespace graphics;

struct App : UniverseApplication 
{
	void on_hud() override
	{
		render_fence->wait();

		hud->begin("test"_h, vec2(100.f));
		static bool v = false;
		if (hud->checkbox(&v, L"AAA"))
			;
		hud->text(L"123456789");
		hud->end();
	}
}app;

int entry(int argc, char** args)
{
	UniverseApplicationOptions app_options;
	app_options.graphics_debug = true;
	app_options.graphics_configs = { {"mesh_shader"_h, 0} };
	app_options.use_tween = false;
	app_options.use_scene = false;
	app_options.use_renderer = false;
	app_options.use_audio = false;
	app_options.use_graveyard = false;
	app.create("Hud Test", uvec2(800, 600), WindowStyleFrame, app_options);

	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
