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
	auto w = new GraphicsWindow(&g_app, true, "Basketball Shooting", Vec2u(600, 400), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(Vec4c(100, 100, 200, 255));
	w->canvas->bind_model(Model::create(L"assets/sm_machine.fm"), "sm_machine.fm");
	root = w->root;

	auto e = Entity::create();
	e->load(L"assets/scene");
	w->s_renderer->set_camera(e->find_child("camera")->get_component_t<cCamera>());
	root->add_child(e);

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = INVALID_POINTER;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
