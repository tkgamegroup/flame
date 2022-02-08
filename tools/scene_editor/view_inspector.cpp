#include "selection.h"
#include "view_inspector.h"

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/model.h>

View_Inspector view_inspector;

View_Inspector::View_Inspector() :
	View("Inspector")
{
}

void show_udt_attributes(const UdtInfo& ui, void* src)
{
	for (auto& a : ui.attributes)
	{
		auto normal_io = a.normal_io();
		switch (a.type->tag)
		{
		case TagD:
		{
			auto ti = (TypeInfo_Data*)a.type;
			switch (ti->data_type)
			{
			case DataBool:
				if (ImGui::Checkbox(a.name.c_str(), (bool*)a.get_value(src, !normal_io)) && !normal_io)
					a.set_value(src);
				break;
			case DataInt:
				switch (ti->vec_size)
				{
				case 1:
					if (ImGui::DragInt(a.name.c_str(), (int*)a.get_value(src, !normal_io)) && !normal_io)
						a.set_value(src);
					break;
				}
				break;
			case DataFloat:
				switch (ti->vec_size)
				{
				case 1:
					if (ImGui::DragFloat(a.name.c_str(), (float*)a.get_value(src, !normal_io)) && !normal_io)
						a.set_value(src);
					break;
				case 2:
					if (ImGui::DragFloat2(a.name.c_str(), (float*)a.get_value(src, !normal_io)) && !normal_io)
						a.set_value(src);
					break;
				case 3:
					if (ImGui::DragFloat3(a.name.c_str(), (float*)a.get_value(src, !normal_io)) && !normal_io)
						a.set_value(src);
					break;
				case 4:
					if (ImGui::DragFloat4(a.name.c_str(), (float*)a.get_value(src, !normal_io)) && !normal_io)
						a.set_value(src);
					break;
				}
				break;
			case DataString:
				if (ImGui::InputText(a.name.c_str(), (std::string*)a.get_value(src, !normal_io)) && !normal_io)
					a.set_value(src);
				break;
			case DataWString:
				break;
			case DataPath:
			{
				auto str = ((std::filesystem::path*)a.get_value(src, !normal_io))->string();
				if (ImGui::InputText(a.name.c_str(), &str) && !normal_io)
				{
					auto path = std::filesystem::path(str);
					a.set_value(src, &path);
				}
				ImGui::SameLine();
				if (ImGui::Button(".."))
				{

				}
			}
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

	switch (selection.type)
	{
	case Selection::tEntity:
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
		break;
	case Selection::tFile:
	{
		ImGui::TextUnformatted(selection.path.string().c_str());
		auto ext = selection.path.extension();
		if (ext == L".obj" || ext == L".fbx" || ext == L".gltf" || ext == L".glb")
		{
			if (ImGui::Button("Convert"))
				graphics::Model::convert(selection.path);
		}
	}
		break;
	}
}
