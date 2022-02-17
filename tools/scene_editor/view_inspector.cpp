#include "app.h"
#include "selection.h"
#include "view_inspector.h"

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/model.h>

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
				auto str = ((std::filesystem::path*)a.get_value(src, !direct_io))->string();
				if (ImGui::InputText(a.name.c_str(), &str))
				{
					if (!direct_io)
					{
						auto path = std::filesystem::path(str);
						a.set_value(src, &path);
					}
					changed_attribute = &a;
				}
				ImGui::SameLine();

				#ifdef USE_IM_FILE_DIALOG
				static const Attribute* dialog_tar = nullptr;
				if (ImGui::Button(("..##" + ::str(src)).c_str()))
				{
					dialog_tar = &a;
					ifd::FileDialog::Instance().Open("PathAttribute", a.name, "*.*");
				}
				if (dialog_tar == &a && ifd::FileDialog::Instance().IsDone("PathAttribute"))
				{
					if (ifd::FileDialog::Instance().HasResult())
					{
						auto path = ifd::FileDialog::Instance().GetResultFormated();
						a.set_value(src, &path);
						changed_attribute = &a;
					}
					ifd::FileDialog::Instance().Close();
				}
				#endif
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

	switch (selection.type)
	{
	case Selection::tEntity:
	{
		auto e = selection.entity;

		auto changed_attribute = show_udt_attributes(*TypeInfo::get<Entity>()->retrive_ui(), e);
		if (changed_attribute)
		{
			if (auto ins = get_prefab_instance(e); ins)
				ins->set_modifier(e->file_id, 0, sh(changed_attribute->name.c_str()), changed_attribute->serialize(e));
		}

		for (auto& c : e->components)
		{
			auto& ui = *com_udts[c->type_hash];
			if (ImGui::CollapsingHeader(ui.name.c_str()))
			{
				auto changed_attribute = show_udt_attributes(ui, c.get());
				if (changed_attribute)
				{
					if (auto ins = get_prefab_instance(e); ins)
						ins->set_modifier(e->file_id, c->type_hash, sh(changed_attribute->name.c_str()), changed_attribute->serialize(e));
				}
			}
		}

		auto str_add_component = "Add Component";
		if (ImGui::Button(str_add_component))
		{
			if (get_prefab_instance(e))
				app.show_message_dialog("[RestructurePrefabInstanceWarnning]");
			else
				ImGui::OpenPopup(str_add_component);
		}
		if (ImGui::BeginPopup(str_add_component))
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
