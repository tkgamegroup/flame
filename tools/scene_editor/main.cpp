#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

App app;

int main(int argc, char** args)
{
	app.create();
	app.set_main_window(Window::create(nullptr, NativeWindow::create(L"Scene Editor", uvec2(1280, 720), NativeWindowFrame | NativeWindowResizable)));

	//auto script_ins = script::Instance::get_default();
	//script_ins->excute_file(L"camera.lua");
	//script_ins->excute_file(L"cmd.lua");
	//script_ins->excute_file(L"main.lua");


	run([](Capture& c, float) {
		app.update();
	}, Capture());

	return 0;
}
