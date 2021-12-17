#include "selection.h"
#include "window_inspector.h"

WindowInspector window_inspector;

WindowInspector::WindowInspector() :
	Window("Inspector")
{
}

void WindowInspector::on_draw()
{
	if (selection.type == Selection::tEntity)
	{
		//auto e = selection.entity;
		//e->get_components();
	}
}
