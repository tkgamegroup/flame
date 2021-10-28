#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char** args)
{
	auto w = NativeWindow::create(L"Window Test", uvec2(1280, 720), NativeWindowFrame);
	w->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
		c.thiz<NativeWindow>()->release();
	}, Capture().set_thiz(w));

	run([](Capture&, float delta_time) {
	}, Capture());

	return 0;
}
