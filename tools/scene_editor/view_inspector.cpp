#include "selection.h"
#include "view_inspector.h"

#include <flame/foundation/typeinfo.h>

View_Inspector view_inspector;

View_Inspector::View_Inspector() :
	View("Inspector")
{
}

void show_udt_attributes(const UdtInfo& ui, void* src)
{
	for (auto& vi : ui.variables)
	{
		auto p = (char*)src + vi.offset;

		switch (vi.type->tag)
		{
		case TagD:
		{
			auto ti = (TypeInfo_Data*)vi.type;
			switch (ti->data_type)
			{

			case DataString:
				ImGui::InputText(vi.name.c_str(), (std::string*)p);
				break;
			}
		}
			break;
		}
	}
}

void View_Inspector::on_draw()
{
	if (selection.type == Selection::tEntity)
	{
		auto ui = ((TypeInfo_Udt*)TypeInfo::get<Entity>())->ui;
		show_udt_attributes(*ui, selection.entity);
	}
}
