#include "app.h"
#include "scene_editor/scene_editor.h"

void MyApp::create()
{
	App::create();
}

int main(int argc, char **args)
{
	app.create();

	new SceneEditorWindow;

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
