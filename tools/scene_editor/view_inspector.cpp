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
	for (auto& a : ui.attributes)
	{
		switch (a.type->tag)
		{
		case TagD:
		{
			auto ti = (TypeInfo_Data*)a.type;
			switch (ti->data_type)
			{
			case DataBool:
				if (a.getter_idx == -1 && a.setter_idx == -1)
				{
					auto p = (char*)src + ui.variables[a.var_idx].offset;
					ImGui::Checkbox(a.name.c_str(), (bool*)p);
				}
				else
				{
					bool v;
					if (a.getter_idx != -1)
						v = ui.functions[a.getter_idx].call<bool>(src);
					else
						v = *(bool*)((char*)src + ui.variables[a.var_idx].offset);
					if (ImGui::Checkbox(a.name.c_str(), &v))
					{
						if (a.setter_idx != -1)
							ui.functions[a.setter_idx].call<void>(src, v);
					}
				}
				break;
			case DataFloat:
				switch (ti->vec_size)
				{
				case 1:
					if (a.getter_idx == -1 && a.setter_idx == -1)
					{
						auto p = (char*)src + ui.variables[a.var_idx].offset;
						ImGui::DragFloat(a.name.c_str(), (float*)p);
					}
					else
					{
						float v;
						if (a.getter_idx != -1)
							v = ui.functions[a.getter_idx].call<float>(src);
						else
							v = *(bool*)((char*)src + ui.variables[a.var_idx].offset);
						if (ImGui::DragFloat(a.name.c_str(), &v))
						{
							if (a.setter_idx != -1)
								ui.functions[a.setter_idx].call<void>(src, v);
						}
					}
					break;
				case 2:
					if (a.getter_idx == -1 && a.setter_idx == -1)
					{
						auto p = (char*)src + ui.variables[a.var_idx].offset;
						ImGui::DragFloat2(a.name.c_str(), (float*)p);
					}
					else
					{
						vec2 v;
						if (a.getter_idx != -1)
							v = ui.functions[a.getter_idx].call<vec2>(src);
						else
							v = *(vec2*)((char*)src + ui.variables[a.var_idx].offset);
						if (ImGui::DragFloat2(a.name.c_str(), (float*)&v))
						{
							if (a.setter_idx != -1)
								ui.functions[a.setter_idx].call<void, const vec2&>(src, v);
						}
					}
					break;
				case 3:
					if (a.getter_idx == -1 && a.setter_idx == -1)
					{
						auto p = (char*)src + ui.variables[a.var_idx].offset;
						ImGui::DragFloat3(a.name.c_str(), (float*)p);
					}
					else
					{
						vec3 v;
						if (a.getter_idx != -1)
							v = ui.functions[a.getter_idx].call<vec3>(src);
						else
							v = *(vec3*)((char*)src + ui.variables[a.var_idx].offset);
						if (ImGui::DragFloat3(a.name.c_str(), (float*)&v))
						{
							if (a.setter_idx != -1)
								ui.functions[a.setter_idx].call<void, const vec3&>(src, v);
						}
					}
					break;
				case 4:
					if (a.getter_idx == -1 && a.setter_idx == -1)
					{
						auto p = (char*)src + ui.variables[a.var_idx].offset;
						ImGui::DragFloat4(a.name.c_str(), (float*)p);
					}
					else
					{
						vec4 v;
						if (a.getter_idx != -1)
							v = ui.functions[a.getter_idx].call<vec4>(src);
						else
							v = *(vec4*)((char*)src + ui.variables[a.var_idx].offset);
						if (ImGui::DragFloat4(a.name.c_str(), (float*)&v))
						{
							if (a.setter_idx != -1)
								ui.functions[a.setter_idx].call<void, const vec4&>(src, v);
						}
					}
					break;
				}
				break;
			case DataString:
				if (a.getter_idx == -1 && a.setter_idx == -1)
				{
					auto p = (char*)src + ui.variables[a.var_idx].offset;
					ImGui::InputText(a.name.c_str(), (std::string*)p);
				}
				else
				{
					std::string v;
					if (a.getter_idx != -1)
						v = ui.functions[a.getter_idx].call<std::string>(src);
					else
						v = *(std::string*)((char*)src + ui.variables[a.var_idx].offset);
					if (ImGui::InputText(a.name.c_str(), &v))
					{
						if (a.setter_idx != -1)
							ui.functions[a.setter_idx].call<void, const std::string&>(src, v);
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
