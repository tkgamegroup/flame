#include "selection.h"
#include "view_inspector.h"

View_Inspector view_inspector;

View_Inspector::View_Inspector() :
	View("Inspector")
{
}

void View_Inspector::on_draw()
{
	if (selection.type == Selection::tEntity)
	{
		//auto e = selection.entity;
		//e->get_components();
	}
}
