#include "app.h"
#include "selection.h"
#include "view_inspector.h"

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/model.h>
#include <flame/graphics/animation.h>
#include <flame/universe/components/armature.h>

View_Inspector view_inspector;

View_Inspector::View_Inspector() :
	View("Inspector")
{
}

const Attribute* show_udt_attributes(const UdtInfo& ui, void* src)
{
	const Attribute* changed_attribute = nullptr;

	for (auto& a : ui.attributes)
	{
		auto direct_io = a.getter_idx == -1 && a.setter_idx == -1;

		switch (a.type->tag)
		{
		case TagD:
		{
			auto ti = (TypeInfo_Data*)a.type;
			switch (ti->data_type)
			{
			case DataBool:
				if (ImGui::Checkbox(a.name.c_str(), (bool*)a.get_value(src, !direct_io)))
				{
					if (!direct_io)
						a.set_value(src);
					changed_attribute = &a;
				}
				break;
			case DataInt:
				switch (ti->vec_size)
				{
				case 1:
					if (ImGui::DragInt(a.name.c_str(), (int*)a.get_value(src, !direct_io)))
					{
						if (!direct_io)
							a.set_value(src);
						changed_attribute = &a;
					}
					break;
				}
				break;
			case DataFloat:
				switch (ti->vec_size)
				{
				case 1:
					if (ImGui::DragFloat(a.name.c_str(), (float*)a.get_value(src, !direct_io)))
					{
						if (!direct_io)
							a.set_value(src);
						changed_attribute = &a;
					}
					break;
				case 2:
					if (ImGui::DragFloat2(a.name.c_str(), (float*)a.get_value(src, !direct_io)))
					{
						if (!direct_io)
							a.set_value(src);
						changed_attribute = &a;
					}
					break;
				case 3:
					if (ImGui::DragFloat3(a.name.c_str(), (float*)a.get_value(src, !direct_io)))
					{
						if (!direct_io)
							a.set_value(src);
						changed_attribute = &a;
					}
					break;
				case 4:
					if (ImGui::DragFloat4(a.name.c_str(), (float*)a.get_value(src, !direct_io)))
					{
						if (!direct_io)
							a.set_value(src);
						changed_attribute = &a;
					}
					break;
				}
				break;
			case DataString:
				if (ImGui::InputText(a.name.c_str(), (std::string*)a.get_value(src, !direct_io)))
				{
					if (!direct_io)
						a.set_value(src);
					changed_attribute = &a;
				}
				break;
			case DataWString:
				break;
			case DataPath:
			{
				auto& path = *(std::filesystem::path*)a.get_value(src, !direct_io);
				auto s = path.string();
				ImGui::InputText(a.name.c_str(), s.data(), ImGuiInputTextFlags_ReadOnly);
				if (ImGui::BeginDragDropTarget())
				{
					if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
					{
						auto str = std::wstring((wchar_t*)payload->Data);
						auto path = Path::reverse(str);
						a.set_value(src, &path);
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				if (ImGui::Button(("P##" + str(&a)).c_str()))
					selection.select(Path::get(path));
			}
				break;
			}
		}
			break;
		}
	}

	return changed_attribute;
}

static std::unordered_map<uint, UdtInfo*> com_udts;
void get_com_udts()
{
	for (auto& ui : tidb.udts)
	{
		if (ui.second.base_class_name == "flame::Component")
			com_udts.emplace(ui.first, &ui.second);
	}
}

void View_Inspector::on_draw()
{
	if (com_udts.empty())
		get_com_udts();

	if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-left"_h).c_str()))
		selection.backward();
	ImGui::SameLine();
	if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-right"_h).c_str()))
		selection.forward();

	switch (selection.type)
	{
	case Selection::tEntity:
	{
		auto e = selection.entity();

		ImGui::PushID(e);
		auto changed_attribute = show_udt_attributes(*TypeInfo::get<Entity>()->retrive_ui(), e);
		ImGui::PopID();
		if (changed_attribute)
		{
			if (auto ins = get_prefab_instance(e); ins)
				ins->set_modifier(e->file_id, "", changed_attribute->name, changed_attribute->serialize(e));
		}

		ComponentPtr com_menu_tar = nullptr;
		for (auto& c : e->components)
		{
			ImGui::PushID(c.get());
			auto& ui = *com_udts[c->type_hash];
			auto open = ImGui::CollapsingHeader(ui.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
			if (ImGui::Button("..."))
				com_menu_tar = c.get();
			if (open)
			{
				auto changed_attribute = show_udt_attributes(ui, c.get());
				if (changed_attribute)
				{
					if (auto ins = get_prefab_instance(e); ins)
						ins->set_modifier(e->file_id, ui.name, changed_attribute->name, changed_attribute->serialize(c.get()));
				}

				if (ui.name == "flame::cArmature")
				{
					auto armature = (cArmaturePtr)c.get();
					if (ImGui::Button("Bind Animation"))
					{
						app.open_input_dialog("Name to bind", [armature](bool ok, const std::string& text) {
							if (ok)
							{

							}
						});
					}
					static char name[100];
					ImGui::InputText("name", name, countof(name));
					ImGui::SameLine();
					if (ImGui::Button("Play"))
						armature->play(sh(name));
					ImGui::SameLine();
					if (ImGui::Button("Stop"))
						armature->stop();
				}
			}
			ImGui::PopID();
		}

		ImGui::Dummy(vec2(0.f, 10.f));
		const float ButtonWidth = 100.f;
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ButtonWidth) * 0.5f);
		ImGui::SetNextItemWidth(ButtonWidth);
		if (ImGui::Button("Add Component"))
		{
			if (get_prefab_instance(e))
				app.open_message_dialog("[RestructurePrefabInstanceWarnning]");
			else
				ImGui::OpenPopup("add_component");
		}

		if (com_menu_tar)
			ImGui::OpenPopup("component_menu");
		if (ImGui::BeginPopup("component_menu"))
		{
			if (ImGui::Selectable("Remove"))
				e->remove_component(com_menu_tar->type_hash);
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("add_component"))
		{
			for (auto ui : com_udts)
			{
				if (ImGui::Selectable(ui.second->name.c_str()))
					e->add_component(ui.first);
			}
			ImGui::EndPopup();
		}
	}
		break;
	case Selection::tPath:
	{
		auto& path = selection.path();
		ImGui::TextUnformatted(Path::reverse(path).string().c_str());
		auto ext = path.extension();
		if (ext == L".obj" || ext == L".fbx" || ext == L".gltf" || ext == L".glb")
		{
			static vec3 rotation = vec3(0, 0, 0);
			static vec3 scaling = vec3(0.01f, 0.01f, 0.01f);
			static bool only_animation = false;
			ImGui::DragFloat3("rotation", (float*)&rotation);
			ImGui::DragFloat3("scaling", (float*)&scaling);
			ImGui::Checkbox("only animation", &only_animation);
			if (ImGui::Button("Convert"))
				graphics::Model::convert(path, rotation, scaling, only_animation);
		}
	}
		break;
	}
}
