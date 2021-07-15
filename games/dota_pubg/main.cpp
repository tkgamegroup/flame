#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App g_app;

int main(int argc, char** args)
{
	g_app.create();

	auto w = new GraphicsWindow(&g_app, L"Dota Pubg", uvec2(800, 600), WindowFrame | WindowResizable, true);
	w->window->set_cursor(CursorNone);
	w->s_renderer->set_shadow_props(3, 50.f, 20.f);

	auto script_ins = script::Instance::get_default();
	script_ins->excute_file(L"terrain_scatter.lua");
	script_ins->excute_file(L"character.lua");
	script_ins->excute_file(L"player.lua");
	script_ins->excute_file(L"npc.lua");
	script_ins->excute_file(L"skill.lua");
	script_ins->excute_file(L"item.lua");
	script_ins->excute_file(L"projectile.lua");
	{
		auto e = Entity::create();
		e->load(L"main");
		w->root->add_child(e);
	}
	script_ins->excute_file(L"main.lua");

	//w->s_physics->set_visualization(true);

	looper().add_event([](Capture& c) {
		printf("fps: %d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
