#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char** args)
{
	auto w = Window::create("Window Test", Vec2u(1280, 720), WindowFrame);

	w->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
		if (is_mouse_down(action, key))
			(*((WindowPtr*)c))->close();
	}, new_mail(&w));

	looper().loop([](void* c) {
	}, Mail());

	return 0;
}
