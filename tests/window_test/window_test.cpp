#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char** args)
{
	auto w = Window::create("Window Test", Vec2u(1280, 720), WindowFrame);
	w->add_mouse_listener([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
		if (is_mouse_down(action, key))
			c.thiz<Window>()->close();
	}, Capture().set_thiz(w));

	looper().loop([](Capture&) {
	}, Capture());

	return 0;
}
