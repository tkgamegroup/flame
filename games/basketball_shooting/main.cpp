#include <flame/graphics/model.h>
#include <flame/universe/app.h>
#include <flame/universe/components/camera.h>

using namespace flame;
using namespace graphics;

App g_app;

Entity* root;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, "Basketball Shooting", uvec2(600, 400), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(cvec4(100, 100, 200, 255));
	w->canvas->bind_model(Model::create(L"D:/basketball_shooting_machine/machine.fmod"), "machine.fmod");
	root = w->root;

	auto e = Entity::create();
	e->load(L"assets/scene");
	w->s_renderer->set_camera(e->find_child("camera")->get_component_t<cCamera>());
	root->add_child(e);

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
