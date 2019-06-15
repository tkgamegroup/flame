#include <flame/foundation/window.h>

using namespace flame;

int main(int argc, char** args)
{
	auto app = Application::create();
	auto w = Window::create(app, "Window Test", Ivec2(1280, 720), WindowFrame);

	w->add_mouse_listener(Function<void(void*, KeyState, MouseKey, const Ivec2&)>(
		[](void* c, KeyState action, MouseKey key, const Ivec2 & pos) {
			if (is_mouse_down(action, key))
				(*((WindowPtr*)c))->close();
		}, sizeof(void*), &w));

	app->run(Function<void(void* c)>(
		[](void* c) {
		}));

	return 0;
}
