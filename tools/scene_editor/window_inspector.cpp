#include "window_inspector.h"
#include "window_scene.h"

WindowInspector window_inspector;

WindowInspector::WindowInspector() :
	Window("Inspector")
{
}

void WindowInspector::on_draw()
{
	auto e = window_scene.e_prefab;
	while (e)
	{
		auto n = e->get_children_count();
	}
}
