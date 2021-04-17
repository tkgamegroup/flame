#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char** args)
{
	auto w = Window::create(L"Window Test", uvec2(1280, 720), WindowFrame);
	w->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
		c.thiz<Window>()->release();
	}, Capture().set_thiz(w));

	looper().loop([](Capture&, float delta_time) {
	}, Capture());

	return 0;
}
