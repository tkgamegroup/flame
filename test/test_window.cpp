#include <flame/foundation/window.h>
#include <flame/foundation/serialize.h>

using namespace flame;

int main(int argc, char** args)
{
	typeinfo_check_update();

	auto w = Window::create("Window Test", Vec2u(1280, 720), WindowFrame);

	w->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
		if (is_mouse_down(action, key))
			(*((WindowPtr*)c))->close();
	}, new_mail(&w));

	app_run([](void* c) {
	}, Mail());

	return 0;
}
