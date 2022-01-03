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
			case DataBool:
				ImGui::Checkbox(vi.name.c_str(), (bool*)p);
				break;
			case DataFloat:
				switch (ti->vec_size)
				{
				case 1:
					ImGui::DragFloat(vi.name.c_str(), (float*)p);
					break;
				case 2:
					ImGui::DragFloat2(vi.name.c_str(), (float*)p);
					break;
				case 3:
					ImGui::DragFloat3(vi.name.c_str(), (float*)p);
					break;
				case 4:
					ImGui::DragFloat4(vi.name.c_str(), (float*)p);
					break;
				}
				break;
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
	auto get_com_udts = []() {
		std::unordered_map<uint, UdtInfo*> ret;
		for (auto& ui : tidb.udts)
		{
			if (ui.second.base_class_name == "flame::Component")
				ret.emplace(ui.first, &ui.second);
		}
		return ret;
	};
	static std::unordered_map<uint, UdtInfo*> com_udts = get_com_udts();

	if (selection.type == Selection::tEntity)
	{
		show_udt_attributes(*TypeInfo::get<Entity>()->retrive_ui(), selection.entity);

		for (auto& c : selection.entity->components)
		{
			auto& ui = *com_udts[c->type_hash];
			if (ImGui::CollapsingHeader(ui.name.c_str()))
				show_udt_attributes(ui, c.get());
		}

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("Add Component");
		if (ImGui::BeginPopup("Add Component"))
		{
			for (auto ui : com_udts)
			{
				if (ImGui::Selectable(ui.second->name.c_str()))
					selection.entity->add_component(ui.first);
			}
			ImGui::EndPopup();
		}
	}
}
