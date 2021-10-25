#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
	void create();
}g_app;

struct MainForm : GraphicsWindow
{
	MainForm();
};

void MyApp::create()
{
	App::create();
}

MainForm::MainForm() :
	GraphicsWindow(&g_app, L"UI Test", uvec2(1280, 720), WindowFrame | WindowResizable)
{

}

int main(int argc, char** args)
{
	g_app.create();
	new MainForm();
	{
		auto e = Entity::create();
		e->load(L"ui_test");
		g_app.main_window->root->add_child(e);
	}
	g_app.run();

	return 0;
}
