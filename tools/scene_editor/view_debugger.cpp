#include "view_debugger.h"

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/image.h>
#include <flame/graphics/debug.h>

View_Debugger view_debugger;

View_Debugger::View_Debugger() :
	GuiView("Debugger")
{
}

void View_Debugger::on_draw()
{
	if (ImGui::TreeNode("Images"))
	{
		auto all_images = graphics::Debug::get_images();
		for (auto i : all_images)
		{
			if (ImGui::TreeNode(std::format("{} {} {}", str(i), TypeInfo::serialize_t(i->format), str(i->size)).c_str()))
			{
				ImGui::Image(i, (vec2)i->size);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}
