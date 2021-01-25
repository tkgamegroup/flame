#include <flame/universe/app.h>
#include <flame/universe/components/command.h>
#include <flame/universe/components/camera.h>

using namespace flame;
using namespace graphics;

App g_app;

auto test_prefab = std::filesystem::path(L"tests/scene_test");

int main(int argc, char** args)
{
	g_app.create();

	auto w = new GraphicsWindow(&g_app, L"Universe Test", uvec2(800, 600), WindowFrame | WindowResizable, true);

	w->canvas->set_clear_color(cvec4(100, 100, 100, 255));

	{
		auto c = cCommand::create();
		c->add_processor([](Capture& c, const char* _cmd) {
			auto cmd = std::string(_cmd);
			if		(cmd == "shading_solid")
				g_app.main_window->canvas->set_shading(graphics::ShadingSolid);
			else if (cmd == "shading_wireframe")
				g_app.main_window->canvas->set_shading(graphics::ShadingWireframe);
			else if (cmd == "physics_visualization_on")
				g_app.main_window->s_physic_world->set_visualization(true);
			else if (cmd == "physics_visualization_off")
				g_app.main_window->s_physic_world->set_visualization(false);
		}, Capture());
		w->root->add_component(c);
		auto e = Entity::create();
		e->load(test_prefab.c_str());
		//e->save(L"d:/1.prefab");
		w->root->add_child(e);
	}

	looper().add_event([](Capture& c) {
		auto dispatcher = g_app.main_window->s_dispatcher;
		printf("------------------------\n");
		auto hovering = dispatcher->get_hovering();
		auto focusing = dispatcher->get_focusing();
		auto active = dispatcher->get_active();
		printf("hovering: %s, focusing: %s, active: %s\n",
			hovering ? hovering->entity->get_name() : "",
			focusing ? focusing->entity->get_name() : "",
			active ? active->entity->get_name() : "");
		printf("fps: %d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
