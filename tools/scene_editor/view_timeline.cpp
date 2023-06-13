#include "selection.h"
#include "view_timeline.h"

#include <flame/universe/timeline.h>

View_Timeline view_timeline;

View_Timeline::View_Timeline() :
	GuiView("Timeline")
{
}

void View_Timeline::on_draw()
{
	if (app.opened_timeline)
	{
		if (ImGui::Button("Close"))
			app.close_timeline();
		else
		{
			for (auto& s : app.opened_timeline->strips)
			{
				ImGui::Selectable(s.address.c_str());
			}
		}
	}
	else
		ImGui::Text("No Timeline opened, open one in the project view.");
}
