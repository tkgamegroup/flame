#include <flame/foundation/window.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

int main(int argc, char** args)
{
	auto w = NativeWindow::create("Window Test", uvec2(1280, 720), WindowFrame);
	w->add_mouse_left_down_listener([w](const ivec2& pos) {
		w->release();
	});

	run([](Capture&, float delta_time) {
	}, Capture());

	return 0;
}
