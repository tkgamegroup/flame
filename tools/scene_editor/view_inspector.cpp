#include "app.h"
#include "selection.h"
#include "view_inspector.h"

#include <flame/foundation/typeinfo.h>
#include <flame/foundation/typeinfo_serialize.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/material.h>
#include <flame/graphics/model.h>
#include <flame/graphics/animation.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/debug.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/armature.h>
#include <flame/universe/components/terrain.h>
#include <flame/universe/components/volume.h>
#include <flame/universe/components/particle_system.h>

View_Inspector view_inspector;
static auto selection_changed = false;

static std::unordered_map<uint, UdtInfo*> com_udts_map;
static std::vector<UdtInfo*> com_udts_list;

View_Inspector::View_Inspector() :
	GuiView("Inspector")
{
	selection.callbacks.add([](uint caller) {
		if (caller != "inspector"_h)
			selection_changed = true;
	}, "inspector"_h);
}

void View_Inspector::reset()
{
	com_udts_map.clear();
	com_udts_list.clear();
}

struct StagingVector
{
	std::vector<char> v;
	TypeInfo* type = nullptr;
	uint item_size = 0;

	StagingVector(TypeInfo* _type)
	{
		type = _type;
		item_size = type->size;
	}

	~StagingVector()
	{
		if (type)
		{
			auto n = count();
			for (auto i = 0; i < n; i++)
				type->destroy(v.data() + i * item_size, false);
			v.clear();
			type = nullptr;
			item_size = 0;
		}
	}

	uint count()
	{
		return v.size() / item_size;
	}

	void resize(void* _vec, uint size)
	{
		if (!_vec)
			_vec = &v;
		auto& vec = *(std::vector<char>*)_vec;
		auto old_size = vec.size() / item_size;
		v.resize(size * item_size);
		if (old_size < size)
		{
			for (auto i = old_size; i < size; i++)
				type->create(vec.data() + i * item_size);
		}
		else if (old_size > size)
		{
			for (auto i = size; i < old_size; i++)
				type->destroy(vec.data() + i * item_size, false);
		}
	}

	void assign(void* _dst, void* _src)
	{
		if (!_dst)
			_dst = &v;
		if (!_src)
			_src = &v;
		auto& dst = *(std::vector<char>*)_dst;
		auto& src = *(std::vector<char>*)_src;
		dst.resize(src.size());
		auto count = src.size() / item_size;
		for (auto i = 0; i < count; i++)
		{
			auto p = dst.data() + i * item_size;
			type->create(p);
			type->copy(p, src.data() + i * item_size);
		}
	}
};

std::unordered_map<const void*, StagingVector> staging_vectors;

StagingVector& get_staging_vector(const void* id, TypeInfo* type, void* vec)
{
	auto it = staging_vectors.find(id);
	if (it != staging_vectors.end())
		return it->second;
	auto& ret = staging_vectors.emplace(id, type).first->second;
	ret.assign(nullptr, vec);
	return ret;
}

struct CommonComponents
{
	uint type_hash;
	std::vector<ComponentPtr> components;
};

struct EditingEntities
{
	std::vector<EntityPtr> entities;
	std::unordered_map<const void*, uint> sync_states;
	std::vector<CommonComponents> common_components;

	void refresh()
	{
		entities.clear();
		sync_states.clear();
		common_components.clear();

		for (auto e : selection.entities())
			entities.push_back(e);
		if (entities.size() <= 1)
			return;

		static auto& ui_entity = *TypeInfo::get<Entity>()->retrive_ui();
		auto entt0 = entities[0];
		auto process_attribute = [&](const Attribute& a, uint comp_hash) {
			void* obj0 = comp_hash == 0 ? entt0 : (void*)entt0->get_component(comp_hash);
			uint state = 1;

			auto var0 = a.type->create();
			a.type->copy(var0, a.get_value(obj0));
			if (a.type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)a.type;
				switch (ti->data_type)
				{
				case DataBool:
					state = *(bool*)var0 ? 1 : 2;
					for (auto i = 1; i < entities.size(); i++)
					{
						void* obj1 = comp_hash == 0 ? entities[i] : (void*)entities[i]->get_component(comp_hash);
						auto var1 = a.get_value(obj1);
						if (*(bool*)var1)
						{
							if (state == 2)
							{
								state = 0;
								break;
							}
						}
						else
						{
							if (state == 1)
							{
								state = 0;
								break;
							}
						}
					}
					break;
				case DataInt:
				case DataFloat:
					for (auto i = 1; i < entities.size(); i++)
					{
						void* obj1 = comp_hash == 0 ? entities[i] : (void*)entities[i]->get_component(comp_hash);
						auto var1 = a.get_value(obj1);
						auto num_diff = 0;
						for (auto y = 0; y < ti->vec_size; y++)
						{
							if (memcmp((char*)var0 + sizeof(float) * y, (char*)var1 + sizeof(float) * y, sizeof(float)) != 0)
							{
								((char*)state)[y] = 0;
								num_diff++;
							}
						}
					}
					break;
				default:
					for (auto i = 1; i < entities.size(); i++)
					{
						void* obj1 = comp_hash == 0 ? entities[i] : (void*)entities[i]->get_component(comp_hash);
						if (!a.type->compare(var0, a.get_value(obj1)))
						{
							state = 0;
							break;
						}
					}
				}

			}
			a.type->destroy(var0);

			sync_states[&a] = state;
		};

		for (auto& a : ui_entity.attributes)
			process_attribute(a, 0);

		for (auto& comp : entt0->components)
		{
			auto hash = comp->type_hash;
			auto all_have = true;
			for (auto i = 1; i < entities.size(); i++)
			{
				if (!entities[i]->get_component(hash))
				{
					all_have = false;
					break;
				}
			}
			if (all_have)
			{
				auto& cc = common_components.emplace_back();
				cc.type_hash = hash;
				cc.components.resize(entities.size());
				for (auto i = 0; i < entities.size(); i++)
					cc.components[i] = entities[i]->get_component(hash);
			}
		}
		for (auto& cc : common_components)
		{
			auto comp0 = entt0->get_component(cc.type_hash);
			auto& ui_comp = *find_udt(cc.type_hash);
			for (auto& a : ui_comp.attributes)
				process_attribute(a, cc.type_hash);
		}
	}
};
static EditingEntities editing_entities;

uint manipulate_udt(const UdtInfo& ui, voidptr* objs, uint num = 1, const std::function<void(uint)>& cb = {});

std::vector<std::string> before_editing_values;

void add_modify_history(uint attr_hash, const std::string& new_value)
{
	auto& eos = editing_objects_list.top();
	switch (eos.type)
	{
	case 0:
	{
		std::vector<std::filesystem::path> paths;
		paths.assign((std::filesystem::path*)eos.objs, (std::filesystem::path*)eos.objs + eos.num);
		add_history(new AssetModifyHistory(paths, eos.type2, attr_hash, before_editing_values, new_value));
	}
		break;
	case 1:
	{
		std::vector<std::string> ids;
		for (auto i = 0; i < eos.num; i++)
			ids[i] = ((EntityPtr*)eos.objs)[i]->instance_id;
		add_history(new EntityModifyHistory(ids, 0, attr_hash, before_editing_values, new_value));
	}
		break;
	case 2:
	{
		std::vector<std::string> ids;
		for (auto i = 0; i < eos.num; i++)
			ids[i] = ((EntityPtr*)eos.objs)[i]->instance_id;
		add_history(new EntityModifyHistory(ids, eos.type2, attr_hash, before_editing_values, new_value));
	}  
		break;
	}
}

bool manipulate_variable(TypeInfo* type, const std::string& name, uint name_hash, int offset, const FunctionInfo* getter, const FunctionInfo* setter, void* obj, const void* id)
{
	auto display_name = get_display_name(name);
	auto changed = false;
	static const void* last_changed = nullptr;
	auto just_exit_editing = false;
	auto direct_io = !getter && !setter;

	ImGui::PushID(id);
	switch (type->tag)
	{
	case TagE:
	{
		auto ti = (TypeInfo_Enum*)type;
		auto ei = ti->ei;
		int value = *(int*)type->get_value(obj, offset, getter);
		auto old_value = value;
		if (!ei->is_flags)
		{
			if (ImGui::BeginCombo(display_name.c_str(), ei->items[value].name.c_str()))
			{
				for (auto& ii : ei->items)
				{
					if (ImGui::Selectable(ii.name.c_str()))
					{
						if (value != ii.value)
						{
							value = ii.value;
							changed = true;
						}
					}
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			std::string str_value;
			for (auto& ii : ei->items)
			{
				if (value == 0 && ii.value == 0)
					str_value = ii.name;
				else if (value & ii.value)
				{
					if (!str_value.empty())
						str_value += '|';
					str_value += ii.name;
				}
			}
			if (ImGui::BeginCombo(display_name.c_str(), str_value.c_str()))
			{
				for (auto& ii : ei->items)
				{
					bool selected = value & ii.value;
					if (ImGui::Selectable(ii.name.c_str(), selected))
					{
						if (selected)
							value &= ~ii.value;
						else
							value |= ii.value;
						changed = true;
					}
				}
				ImGui::EndCombo();
			}
		}
		if (changed)
		{
			if (!direct_io)
				type->set_value(obj, offset, setter, &value);
			if (!editing_objects_list.empty())
			{
				before_editing_values.resize(1);
				before_editing_values[0] = str(old_value);
				add_modify_history(name_hash, str(value));
			}
		}
	}
		break;
	case TagD:
	{
		auto ti = (TypeInfo_Data*)type;
		auto vec_size = ti->vec_size;
		auto data = type->get_value(obj, offset, getter, !direct_io);
		switch (ti->data_type)
		{
		case DataBool:
		{
			auto old_value = *(bool*)data;
			changed = ImGui::Checkbox(display_name.c_str(), (bool*)data);
			if (changed)
			{
				if (!direct_io)
					type->set_value(obj, offset, setter, data);
				if (!editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(old_value);
					add_modify_history(name_hash, str(*(bool*)data));
				}
			}
		}
			break;
		case DataInt:
			switch (ti->vec_size)
			{
			case 1:

				changed = ImGui::InputInt(display_name.c_str(), (int*)data);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(int*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				changed = changed && just_exit_editing;
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(int*)data));
				break;
			case 2:
				changed = ImGui::InputInt2(display_name.c_str(), (int*)data);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(ivec2*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				changed = changed && just_exit_editing;
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(ivec2*)data));
				break;
			case 3:
				changed = ImGui::InputInt3(display_name.c_str(), (int*)data);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(ivec3*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				changed = changed && just_exit_editing;
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(ivec3*)data));
				break;
			case 4:
				changed = ImGui::InputInt4(display_name.c_str(), (int*)data);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(ivec4*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				changed = changed && just_exit_editing;
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(ivec4*)data));
				break;
			}
		break;
		case DataFloat:
			switch (ti->vec_size)
			{
			case 1:
				changed = ImGui::DragFloat(display_name.c_str(), (float*)data, 0.01f);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(float*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(float*)data));
				break;
			case 2:
				changed = ImGui::DragFloat2(display_name.c_str(), (float*)data, 0.01f);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(vec2*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(vec2*)data));
				break;
			case 3:
				changed = ImGui::DragFloat3(display_name.c_str(), (float*)data, 0.01f);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(vec3*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(vec3*)data));
				break;
			case 4:
				changed = ImGui::DragFloat4(display_name.c_str(), (float*)data, 0.01f);
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(vec4*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(vec4*)data));
				break;
			}
			break;
		case DataChar:
			switch (ti->vec_size)
			{
			case 4:
			{
				vec4 color = *(cvec4*)data;
				color /= 255.f;
				changed = ImGui::ColorEdit4(display_name.c_str(), &color[0]);
				if (changed)
					*(cvec4*)data = color * 255.f;
				if (ImGui::IsItemActivated() && !editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = str(*(cvec4*)data);
					last_changed = nullptr;
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
					add_modify_history(name_hash, str(*(cvec4*)data));
			}
				break;
			}
			break;
		case DataString:
			changed = ImGui::InputText(display_name.c_str(), (std::string*)data);
			if (ImGui::IsItemActivated() && !editing_objects_list.empty())
			{
				before_editing_values.resize(1);
				before_editing_values[0] = *(std::string*)data;
				last_changed = nullptr;
			}
			just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
			changed = changed && just_exit_editing;
			if ((changed || last_changed == id) && just_exit_editing && !editing_objects_list.empty())
				add_modify_history(name_hash, *(std::string*)data);
			break;
		case DataWString:
			break;
		case DataPath:
		{
			auto& path = *(std::filesystem::path*)data;
			auto s = path.string();
			ImGui::InputText(display_name.c_str(), s.data(), ImGuiInputTextFlags_ReadOnly);
			if (ImGui::BeginDragDropTarget())
			{
				if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
				{
					if (!editing_objects_list.empty())
					{
						before_editing_values.resize(1);
						before_editing_values[0] = path.string();
					}
					path = Path::reverse(std::wstring((wchar_t*)payload->Data));
					changed = true;
					add_modify_history(name_hash, path.string());
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			if (ImGui::Button("P"))
			{
				add_event([path]() {
					selection.select(Path::get(path), "app"_h);
					return false;
				});
			}
			ImGui::SameLine();
			if (ImGui::Button("X"))
			{
				if (!editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					before_editing_values[0] = path.string();
				}
				path = L"";
				changed = true;
				add_modify_history(name_hash, path.string());
			}
		}
			break;
		}
		if (changed && !direct_io)
			type->set_value(obj, offset, setter, nullptr);
	}
		break;
	case TagVD:
		if (ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)obj + offset;
			auto ti = ((TypeInfo_VectorOfData*)type)->ti;
			auto& sv = get_staging_vector(id, ti, pv);
			if (ImGui::Button("Get"))
				sv.assign(nullptr, pv);
			ImGui::SameLine();
			if (ImGui::Button("Set"))
			{
				if (!setter)
					sv.assign((char*)obj + offset, nullptr);
				else
					type->set_value(obj, offset, setter, &sv.v);
			}
			int n = sv.count();
			if (ImGui::InputInt("size", &n, 1, 1))
			{
				n = clamp(n, 0, 16);
				sv.resize(nullptr, n);
			}
			for (auto i = 0; i < n; i++)
			{
				ImGui::PushID(i);
				if (manipulate_variable(ti, str(i), 0, i * ti->size, nullptr, nullptr, { sv.v.data() }, id))
					changed = true;
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		break;
	case TagVU:
		if (ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)obj + offset;
			auto ti = ((TypeInfo_VectorOfUdt*)type)->ti;
			auto& sv = get_staging_vector(id, ti, pv);
			if (ImGui::Button("Get"))
				sv.assign(nullptr, pv);
			ImGui::SameLine();
			if (ImGui::Button("Set"))
			{
				if (!setter)
					sv.assign((char*)obj + offset, nullptr);
				else
					type->set_value(obj, offset, setter, &sv.v);
			}
			auto& ui = *ti->retrive_ui();
			int n = sv.count();
			if (ImGui::InputInt("size", &n, 1, 1))
			{
				n = clamp(n, 0, 16);
				sv.resize(nullptr, n);
			}
			for (auto i = 0; i < n; i++)
			{
				if (i > 0) ImGui::Separator();
				ImGui::PushID(i);
				voidptr obj = sv.v.data() + ui.size * i;
				manipulate_udt(ui, &obj);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		break;
	case TagVR:
		if (ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)obj + offset;
			auto ti = ((TypeInfo_VectorOfPair*)type)->ti;
			auto& sv = get_staging_vector(id, ti, pv);
			if (ImGui::Button("Get"))
				sv.assign(nullptr, pv);
			ImGui::SameLine();
			if (ImGui::Button("Set"))
			{
				if (!setter)
					sv.assign(pv, nullptr);
				else
					type->set_value(obj, offset, setter, &sv.v);
			}
			int n = sv.count();
			if (ImGui::InputInt("size", &n, 1, 1))
			{
				n = clamp(n, 0, 16);
				sv.resize(nullptr, n);
			}
			for (auto i = 0; i < n; i++)
			{
				if (i > 0) ImGui::Separator();
				ImGui::PushID(i);
				auto p = sv.v.data() + ti->size * i;
				manipulate_variable(ti->ti1, "First", 0, 0, nullptr, nullptr, { ti->first(p) }, id);
				manipulate_variable(ti->ti2, "Second", 0, 0, nullptr, nullptr, { ti->second(p) }, id);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		break;
	case TagVT:
		if (ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)obj + offset;
			auto ti = ((TypeInfo_VectorOfTuple*)type)->ti;
			auto& sv = get_staging_vector(id, ti, pv);
			if (ImGui::Button("Get"))
				sv.assign(nullptr, pv);
			ImGui::SameLine();
			if (ImGui::Button("Set"))
			{
				if (!setter)
					sv.assign((char*)obj + offset, nullptr);
				else
					type->set_value(obj, offset, setter, &sv.v);
			}
			int n = sv.count();
			if (ImGui::InputInt("size", &n, 1, 1))
			{
				n = clamp(n, 0, 16);
				sv.resize(nullptr, n);
			}
			for (auto i = 0; i < n; i++)
			{
				if (i > 0) ImGui::Separator();
				ImGui::PushID(i);
				auto p = sv.v.data() + ti->size * i;
				auto j = 0;
				for (auto& t : ti->tis)
				{
					manipulate_variable(t.first, "Item " + str(j), 0, 0, nullptr, nullptr, { p + t.second }, id);
					j++;
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		break;
	}
	ImGui::PopID();

	if (changed)
		last_changed = id;
	return changed;
}

bool manipulate_variable(TypeInfo* type, const std::string& name, uint name_hash, int offset, const FunctionInfo* getter, const FunctionInfo* setter, voidptr* objs, uint num, const void* id)
{
	auto display_name = get_display_name(name);
	auto changed = false;
	static const void* last_changed = nullptr;
	auto just_exit_editing = false;

	ImGui::PushID(id);
	switch (type->tag)
	{
	case TagE:
	{
		auto ti = (TypeInfo_Enum*)type;
		auto ei = ti->ei;
		std::vector<int> old_values(objs.size());
		for (auto i = 0; i < objs.size(); i++)
			old_values[i] = *(int*)type->get_value(objs[i], offset, getter);
		auto same = true;
		for (auto i = 1; i < objs.size(); i++)
		{
			if (old_values[i] != old_values[0])
			{
				same = false;
				break;
			}
		}
		int value = old_values[0];
		if (!ei->is_flags)
		{
			if (ImGui::BeginCombo(display_name.c_str(), same ? ei->items[value].name.c_str() : "-"))
			{
				for (auto& ii : ei->items)
				{
					if (ImGui::Selectable(ii.name.c_str()))
					{
						if (value != ii.value)
						{
							value = ii.value;
							changed = true;
						}
					}
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			std::string str_value;
			if (same)
			{
				for (auto& ii : ei->items)
				{
					if (value == 0 && ii.value == 0)
						str_value = ii.name;
					else if (value & ii.value)
					{
						if (!str_value.empty())
							str_value += '|';
						str_value += ii.name;
					}
				}
			}
			else
				str_value = "-";
			if (ImGui::BeginCombo(display_name.c_str(), str_value.c_str()))
			{
				for (auto& ii : ei->items)
				{
					bool selected = value & ii.value;
					if (ImGui::Selectable(ii.name.c_str(), selected))
					{
						if (selected)
							value &= ~ii.value;
						else
							value |= ii.value;
						changed = true;
					}
				}
				ImGui::EndCombo();
			}
		}
		if (changed)
		{
			for (auto i = 0; i < objs.size(); i++)
				type->set_value(objs[i], offset, setter, &value);
			if (!editing_objects_list.empty())
			{
				before_editing_values.resize(1);
				for (auto i = 0; i < objs.size(); i++)
					before_editing_values[i] = str(old_values[i]);
				add_modify_history(name_hash, str(value));
			}
		}
	}
		break;
	case TagD:
	{
		auto ti = (TypeInfo_Data*)type;
		auto vec_size = ti->vec_size;
		switch (ti->data_type)
		{
		case DataBool:
		{
			std::vector<uint> old_values(objs.size());
			for (auto i = 0; i < objs.size(); i++)
				old_values[i] = *(bool*)type->get_value(objs[i], offset, getter);
			auto same = true;
			for (auto i = 1; i < objs.size(); i++)
			{
				if (old_values[i] != old_values[0])
				{
					same = false;
					break;
				}
			}
			if (!same)
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
			bool value = same ? old_values[0] : true;
			changed = ImGui::Checkbox(display_name.c_str(), &value);
			if (!same)
				ImGui::PopItemFlag();
			if (changed)
			{
				for (auto i = 0; i < objs.size(); i++)
					type->set_value(objs[i], offset, setter, &value);
				if (!editing_objects_list.empty())
				{
					before_editing_values.resize(1);
					for (auto i = 0; i < objs.size(); i++)
						before_editing_values[i] = str(old_values[i]);
					add_modify_history(name_hash, str(value));
				}
			}
		}
			break;
		case DataInt:
		{
			std::vector<int> old_values(objs.size() * vec_size);
			for (auto i = 0; i < objs.size(); i++)
			{
				auto data = (int*)type->get_value(objs[i], offset, getter);
				for (auto y = 0; y < vec_size; y++)
					old_values[i * vec_size + y] = data[i];
			}

			ImGui::BeginGroup();
			ImGui::PushID(display_name.c_str());
			ImGui::PushMultiItemsWidths(vec_size, ImGui::CalcItemWidth());
			auto inner_spaceing = ImGui::GetStyle().ItemInnerSpacing.x;
			for (auto y = 0; y < vec_size; y++)
			{
				ImGui::PushID(y);
				if (y > 0)
					ImGui::SameLine(0, inner_spaceing);
				//changed |= ImGui::InputScalar("", ImGuiDataType_S32, p_data);
				ImGui::PopID();
				ImGui::PopItemWidth();
			}
			ImGui::PopID();

			ImGui::SameLine(0.0f, inner_spaceing);
			ImGui::TextEx(display_name.c_str());
			ImGui::EndGroup();
		}
			break;
		}
	}
		break;
	}
	ImGui::PopID();

	if (changed)
		last_changed = id;
	return changed;
}

uint manipulate_udt(const UdtInfo& ui, voidptr* objs, uint num, const std::function<void(uint)>& cb)
{
	uint changed_name = 0;

	if (ui.attributes.empty())
	{
		for (auto& v : ui.variables)
		{
			if (manipulate_variable(v.type, v.name, v.name_hash, v.offset, nullptr, nullptr, objs, num, &v))
				changed_name = v.name_hash;
			if (cb)
				cb(v.name_hash);
		}
	}
	else
	{
		for (auto& a : ui.attributes)
		{
			if (manipulate_variable(a.type, a.name, a.name_hash, a.var_off(),
				a.getter_idx != -1 ? &ui.functions[a.getter_idx] : nullptr,
				a.setter_idx != -1 ? &ui.functions[a.setter_idx] : nullptr,
				objs, num, &a))
				changed_name = a.name_hash;
			if (cb)
				cb(a.name_hash);
		}
	}
	return changed_name;
}

void get_com_udts()
{
	for (auto& ui : tidb.udts)
	{
		if (ui.second.base_class_name == "flame::Component")
			com_udts_map.emplace(ui.first, &ui.second);
	}
	std::vector<UdtInfo*> temp1;
	std::vector<UdtInfo*> temp2;
	for (auto& pair : com_udts_map)
	{
		if (pair.second->name.starts_with("flame::"))
			temp1.push_back(pair.second);
		else
			temp2.push_back(pair.second);
	}
	std::sort(temp1.begin(), temp1.end(), [](const auto& a, const auto& b) {
		return a->name < b->name;
	});
	std::sort(temp2.begin(), temp2.end(), [](const auto& a, const auto& b) {
		return a->name < b->name;
	});
	com_udts_list.insert(com_udts_list.end(), temp1.begin(), temp1.end());
	com_udts_list.insert(com_udts_list.end(), temp2.begin(), temp2.end());
}

void View_Inspector::on_draw()
{
	if (com_udts_map.empty())
		get_com_udts();

	if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-left"_h).c_str()))
		selection.backward();
	ImGui::SameLine();
	if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-right"_h).c_str()))
		selection.forward();

	static void* sel_ref_obj = nullptr;
	static void(*sel_ref_deletor)(void*) = nullptr;
	static auto sel_ref_info = new char[1024];
	if (selection_changed)
	{
		staging_vectors.clear();
		editing_entities.refresh();

		if (sel_ref_deletor && sel_ref_obj)
			sel_ref_deletor(sel_ref_obj);
		sel_ref_deletor = nullptr;
		sel_ref_obj = nullptr;
	}

	switch (selection.type)
	{
	case Selection::tEntity:
	{
		uint changed_name = 0;
		static auto& ui_entity = *TypeInfo::get<Entity>()->retrive_ui();
		auto entity = selection.as_entity();

		editing_objects_list.emplace(EditingObjects(1, 0, editing_entities.entities.data(), editing_entities.entities.size()));

		ImGui::PushID("flame::Entity"_h);
		if (editing_entities.entities.size() == 1 && entity->prefab_instance)
		{
			auto& path = entity->prefab_instance->filename;
			auto str = path.string();
			ImGui::InputText("prefab", str.data(), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("P"))
				selection.select(Path::get(path), "inspector"_h);
		}
		changed_name = manipulate_udt(ui_entity, (voidptr*)editing_entities.entities.data(), editing_entities.entities.size());
		ImGui::PopID();

		editing_objects_list.pop();
		if (changed_name != 0)
		{
			auto& str = ui_entity.find_attribute(changed_name)->name;
			for (auto e : editing_entities.entities)
			{
				if (auto ins = get_prefab_instance(e); ins)
					ins->mark_modifier(e->file_id, "", str);
			}
		}

		static uint target_component = 0;
		auto open_component_menu = false;
		for (auto& cc : editing_entities.common_components)
		{
			editing_objects_list.emplace(EditingObjects(2, cc.type_hash, editing_entities.entities.data(), editing_entities.entities.size()));
			ImGui::PushID(cc.type_hash);
			auto& ui = *com_udts_map[cc.type_hash];
			auto open = ImGui::CollapsingHeader(ui.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40);
			if (ImGui::Button("E"))
			{
				if (!ui.source_file.empty())
					app.open_file_in_vs(ui.source_file);
			}
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				target_component = cc.type_hash;
				open_component_menu = true;
			}
			if (open)
			{
				auto enable = c->enable;
				ImGui::Checkbox("Enable", &enable);
				if (enable != c->enable)
					c->set_enable(enable);

				static bool open_select_hash = false;
				static std::vector<std::string> hash_candidates;
				static const Attribute* op_attr;
				static uint op_obj;
				auto changed_name = manipulate_udt(ui, c, [&ui](uint name) {
					ImGui::PushID(name);
					if (name == "mesh_name"_h)
					{
						ImGui::SameLine();
						if (ImGui::Button("S"))
						{
							auto& ori_name = *(std::filesystem::path*)ui.find_attribute("mesh_name"_h)->get_value(obj);

							static const wchar_t* names[] = {
								L"standard_cube",
								L"standard_sphere",
								L"standard_cylinder"
							};
							auto idx = -1;
							for (auto i = 0; i < countof(names); i++)
							{
								if (ori_name == names[i])
								{
									idx = i;
									break;
								}
							}
							if (idx == -1 || idx + 1 == countof(names))
								idx = 0;
							else
								idx++;
							auto path = std::filesystem::path(names[idx]);
							ui.find_function("set_mesh_name"_h)->call<void, void*>(obj, &path);
						}
					}
					else if (name == "material_name"_h)
					{
						ImGui::SameLine();
						if (ImGui::Button("D"))
						{
							auto path = std::filesystem::path(L"default");
							ui.find_function("set_material_name"_h)->call<void, void*>(obj, &path);
						}

						auto& name = *(std::filesystem::path*)ui.find_attribute("material_name"_h)->get_value(obj);
						if (!name.empty() && name != L"default")
						{
							if (ImGui::TreeNode("##embed"))
							{
								// the material is loaded and registered to renderer
								if (auto material = graphics::Material::get(name); material)
								{
									editing_objects_list.emplace(EditingObjects(0, th<graphics::Material>(), &name, 1));
									manipulate_udt(*TypeInfo::get<graphics::Material>()->retrive_ui(), (voidptr*)&material, 1).empty();
									editing_objects_list.pop();
									graphics::Material::release(material);
								}
								ImGui::TreePop();
							}
						}
					}
					else
					{
						auto& a = *ui.find_attribute(name);
						std::string meta;
						if (a.var_idx != -1)
						{
							if (ui.variables[a.var_idx].metas.get("hash"_h, &meta))
							{
								ImGui::SameLine();
								if (ImGui::Button("S"))
								{
									open_select_hash = true;
									hash_candidates = SUS::split(meta, '|');
									op_attr = &a;
									op_obj = obj;
								}
							}
						}
					}
					ImGui::PopID();
				});
				if (open_select_hash)
				{
					ImGui::OpenPopup("select_hash");
					open_select_hash = false;
				}
				if (ImGui::BeginPopup("select_hash"))
				{
					for (auto& c : hash_candidates)
					{
						if (ImGui::Selectable(c.c_str()))
						{
							uint v = sh(c.c_str());
							op_attr->set_value(op_obj, &v);
						}
					}
					ImGui::EndPopup();
				}
				if (!changed_name.empty())
				{
					if (auto ins = get_prefab_instance(e); ins)
						ins->mark_modifier(e->file_id, ui.name, changed_name);
				}

				if (ui.name_hash == "flame::cNode"_h)
				{
					auto node = (cNodePtr)c.get();
					ImGui::InputFloat4("qut", (float*)&node->qut, "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_pos", (float*)&node->g_pos, "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_rot.x", (float*)&node->g_rot[0], "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_rot.y", (float*)&node->g_rot[1], "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_rot.z", (float*)&node->g_rot[2], "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_scl", (float*)&node->g_scl, "%.3f", ImGuiInputTextFlags_ReadOnly);
				}
				else if (ui.name_hash == "flame::cArmature"_h)
				{
					auto armature = (cArmaturePtr)c.get();
					if (!armature->animation_names.empty())
					{
						static int idx = 0;
						idx = clamp(idx, 0, (int)armature->animation_names.size());
						if (ImGui::BeginCombo("animations", armature->animation_names[idx].second.c_str()))
						{
							for (auto i = 0; i < armature->animation_names.size(); i++)
							{
								auto& name = armature->animation_names[i].second;
								if (ImGui::Selectable(name.c_str()))
								{
									idx = i;
									armature->play(sh(name.c_str()));
								}
							}
							ImGui::EndCombo();
						}
					}
					if (armature->playing_name != 0)
					{
						ImGui::SameLine();
						if (ImGui::Button("Stop"))
							armature->stop();
						ImGui::InputFloat("Time", &armature->playing_time, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
					}
					ImGui::DragFloat("Speed", &armature->playing_speed, 0.01f);
				}
			}
			ImGui::PopID();
			editing_objects_list.pop();
		}

		ImGui::Dummy(vec2(0.f, 10.f));
		const float ButtonWidth = 100.f;
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ButtonWidth) * 0.5f);
		ImGui::SetNextItemWidth(ButtonWidth);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("add_component");

		if (open_component_menu)
		{
			ImGui::OpenPopup("component_menu");
			open_component_menu = false;
		}
		if (ImGui::BeginPopup("component_menu"))
		{
			if (ImGui::Selectable("Move Up"))
				;
			if (ImGui::Selectable("Move Down"))
				;
			if (ImGui::Selectable("Remove"))
			{
				auto ok = true;
				for (auto e : editing_entities.entities)
				{
					if (get_prefab_instance(e))
					{
						ok = false;
						break;
					}
				}
				if (ok)
				{
					for (auto e : editing_entities.entities)
						e->remove_component(target_component);
					editing_entities.refresh();
				}
				else
					app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("add_component"))
		{
			for (auto ui : com_udts_list)
			{
				if (ImGui::Selectable(ui->name.c_str()))
				{
					auto ok = true;
					for (auto e : editing_entities.entities)
					{
						if (get_prefab_instance(e))
						{
							ok = false;
							break;
						}
					}
					if (ok)
					{
						for (auto e : editing_entities.entities)
							e->add_component(ui->name_hash);
						editing_entities.refresh();
					}
					else
						app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
				}
			}
			ImGui::EndPopup();
		}
	}
		break;
	case Selection::tPath:
	{
		if (selection.objects.size() == 1)
		{
			auto path = selection.as_path();
			ImGui::TextUnformatted(Path::reverse(path).string().c_str());
			auto ext = path.extension().wstring();
			SUW::to_lower(ext);
			if (ext == L".obj" || ext == L".fbx" || ext == L".gltf" || ext == L".glb")
			{
				static vec3 rotation = vec3(0, 0, 0);
				static vec3 scaling = vec3(0.01f, 0.01f, 0.01f);
				static bool only_animation = false;
				static bool copy_textures = false;
				static std::string texture_fmt = "";
				ImGui::DragFloat3("Rotation", (float*)&rotation);
				ImGui::DragFloat3("Scaling", (float*)&scaling);
				ImGui::Checkbox("Only Animation", &only_animation);
				ImGui::Checkbox("Copy Textures", &copy_textures);
				if (copy_textures)
					ImGui::InputText("Texture Format", &texture_fmt);
				if (ImGui::Button("Convert"))
					graphics::Model::convert(path, rotation, scaling, only_animation, copy_textures, texture_fmt);
			}
			else if (ext == L".fmod")
			{
				if (ImGui::Button("To Text"))
				{
					auto model = graphics::Model::get(path);
					model->save(path, false);
					graphics::Model::release(model);
				}
				if (ImGui::Button("To Binary"))
				{
					auto model = graphics::Model::get(path);
					model->save(path, true);
					graphics::Model::release(model);
				}
			}
			else if (is_image_file(ext))
			{
				struct ImageRefInfo
				{
					uint chs;
					uint bpp;
					bool srgb;
				};
				auto& info = *(ImageRefInfo*)sel_ref_info;

				if (selection_changed)
				{
					auto image = graphics::Image::get(path);
					if (image)
					{
						sel_ref_obj = image;
						sel_ref_deletor = [](void* obj) {
							graphics::Image::release((graphics::ImagePtr)obj);
						};

						auto bitmap = Bitmap::create(path);
						if (bitmap)
						{
							info.chs = bitmap->chs;
							info.bpp = bitmap->bpp;
							info.srgb = bitmap->srgb;
							delete bitmap;
						}
						else
						{
							info.chs = 0;
							info.bpp = 0;
							info.srgb = false;
						}
					}
				}

				if (sel_ref_obj)
				{
					auto image = (graphics::ImagePtr)sel_ref_obj;
					ImGui::Text("Channels: %d", info.chs);
					ImGui::Text("Bits Per Pixel: %d", info.bpp);
					ImGui::Text("SRGB: %s", info.srgb ? "yes" : "no");
					ImGui::Text("Graphics Format: %s", TypeInfo::serialize_t(image->format).c_str());
					ImGui::Text("Extent: %s", str(image->extent).c_str());
					static int view_type = ImGui::ImageViewRGBA;
					static const char* types[] = {
						"RGBA",
						"R", "G", "B", "A",
						"RGB",
					};
					ImGui::Combo("View", &view_type, types, countof(types));
					if (view_type != 0)
						ImGui::PushImageViewType((ImGui::ImageViewType)view_type);
					if (image->extent.z == 1)
						ImGui::Image(sel_ref_obj, (vec2)image->extent);
					if (view_type != 0)
						ImGui::PopImageViewType();
					if (ImGui::Button("Save"))
					{
						image->save(path);
						auto asset = AssetManagemant::find(path);
						if (asset)
							asset->active = false;
					}
				}
			}
			else if (ext == L".fmat")
			{
				if (selection_changed || !sel_ref_obj)
				{
					auto material = graphics::Material::get(path);
					if (material)
					{
						sel_ref_obj = material;
						sel_ref_deletor = [](void* obj) {
							graphics::Material::release((graphics::MaterialPtr)obj);
						};
					}
				}

				if (sel_ref_obj)
				{
					auto material = (graphics::MaterialPtr)sel_ref_obj;
					editing_objects_list.emplace(new EditingObjects(0, th<graphics::Material>(), &path, 1));
					manipulate_udt(*TypeInfo::get<graphics::Material>()->retrive_ui(), material).empty();
					editing_objects_list.pop();
					if (ImGui::Button("Save"))
					{
						material->save(path);
						auto asset = AssetManagemant::find(path);
						if (asset)
							asset->active = false;
					}
				}
			}
			else if (ext == L".fmod")
			{
				if (selection_changed || !sel_ref_obj)
				{
					auto model = graphics::Model::get(path);
					if (model)
					{
						sel_ref_obj = model;
						sel_ref_deletor = [](void* obj) {
							graphics::Model::release((graphics::ModelPtr)obj);
						};
					}
				}

				if (sel_ref_obj)
				{
					auto model = (graphics::ModelPtr)sel_ref_obj;
					auto i = 0;
					for (auto& mesh : model->meshes)
					{
						ImGui::Text("Mesh %d:", i);
						ImGui::Text("Vertex Count: %d", mesh.positions.size());
						ImGui::Text("Index Count: %d", mesh.indices.size());
						std::string attr_str = "";
						if (!mesh.uvs.empty()) attr_str += " uv";
						if (!mesh.normals.empty()) attr_str += " normal";
						if (!mesh.tangents.empty()) attr_str += " tangent";
						if (!mesh.colors.empty()) attr_str += " color";
						if (!mesh.bone_ids.empty()) attr_str += " bone_ids";
						if (!mesh.bone_weights.empty()) attr_str += " bone_weights";
						ImGui::Text("Attributes: %s", attr_str.data());
						i++;
					}
				}
			}
			else if (ext == L".fani")
			{
				if (selection_changed || !sel_ref_obj)
				{
					auto animation = graphics::Animation::get(path);
					if (animation)
					{
						sel_ref_obj = animation;
						sel_ref_deletor = [](void* obj) {
							graphics::Animation::release((graphics::AnimationPtr)obj);
						};
					}
				}

				if (sel_ref_obj)
				{
					auto animation = (graphics::AnimationPtr)sel_ref_obj;
					ImGui::Text("Duration: %f", animation->duration);
					if (ImGui::TreeNode(std::format("Channels ({})", (int)animation->channels.size()).c_str()))
					{
						ImGui::BeginChild("channels", ImVec2(0, 0), true);
						for (auto& ch : animation->channels)
						{
							if (ImGui::TreeNode(ch.node_name.c_str()))
							{
								if (ImGui::TreeNode(std::format("  Position Keys({})", (int)ch.position_keys.size()).c_str()))
								{
									for (auto& k : ch.position_keys)
										ImGui::Text("    %f: %s", k.t, str(k.p).c_str());
									ImGui::TreePop();
								}
								if (ImGui::TreeNode(std::format("  Rotation Keys({})", (int)ch.rotation_keys.size()).c_str()))
								{
									for (auto& k : ch.rotation_keys)
										ImGui::Text("    %f: %s", k.t, str((vec4&)k.q).c_str());
									ImGui::TreePop();
								}
								ImGui::TreePop();
							}
						}
						ImGui::EndChild();
						ImGui::TreePop();
					}
				}
			}
			else if (ext == L".pipeline")
			{
				static auto ti = TypeInfo::get<graphics::PipelineInfo>();
				static UdtInfo* ui = ti->retrive_ui();
				static UdtInfo* ser_ui = ui->transform_to_serializable();
				static std::vector<std::pair<std::string, std::string>> default_defines;

				if (selection_changed || !sel_ref_obj)
				{
					sel_ref_obj = ser_ui->create_object();
					sel_ref_deletor = [](void* obj) {
						ser_ui->destroy_object(obj);
						default_defines.clear();
					};

					std::ifstream file(path);
					LineReader res(file);
					res.read_block("");
					UnserializeTextSpec spec;
					spec.out_default_defines = &default_defines;
					unserialize_text(*ser_ui, res, 0, sel_ref_obj, spec);
					file.close();
				}

				if (sel_ref_obj)
				{
					manipulate_variable(TypeInfo::get<decltype(default_defines)>(), "default defines", 0, 0, nullptr, nullptr, &default_defines, nullptr);
					manipulate_udt(*ser_ui, { sel_ref_obj });
					if (ImGui::Button("Test Compile"))
					{
						auto pl = graphics::GraphicsPipeline::get(path, {});
						if (pl)
							graphics::GraphicsPipeline::release(pl);
					}
					if (ImGui::Button("Save"))
					{
						std::ofstream file(path);
						for (auto& d : default_defines)
							file << '%' << d.first << '=' << d.second << std::endl;
						SerializeTextSpec spec;
						spec.force_print_bar = true;
						serialize_text(*ser_ui, sel_ref_obj, file, "", spec);
						file.close();
					}
					if (ImGui::Button("Open Sandbox"))
					{
						if (!app.project_path.empty())
						{
							auto temp_path = app.project_path / L"temp";
							auto sandbox_path = temp_path;
							sandbox_path /= path.filename().stem();
							if (!std::filesystem::exists(sandbox_path))
								std::filesystem::create_directories(sandbox_path);
							else
								std::filesystem::remove_all(sandbox_path);

							auto cmake_path = sandbox_path / L"CMakeLists.txt";
							std::ofstream cmake_lists(cmake_path);
							const auto cmake_content =
								R"^^^(
cmake_minimum_required(VERSION 3.16.4)
set(flame_path "$ENV{{FLAME_PATH}}")
set_output_dir("${{CMAKE_SOURCE_DIR}}/bin")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_definitions(-W0 -std:c++latest)

project({0})

set(GLM_INCLUDE_DIR "")
file(STRINGS "${{flame_path}}/build/CMakeCache.txt" flame_cmake_cache)
foreach(s ${{flame_cmake_cache}})
	if(GLM_INCLUDE_DIR STREQUAL "")
		string(REGEX MATCH "GLM_INCLUDE_DIR:PATH=(.*)" res "${{s}}")
		if(NOT res STREQUAL "")
			set(GLM_INCLUDE_DIR ${{CMAKE_MATCH_1}})
		endif()
	endif()
endforeach()

file(GLOB_RECURSE source_files "cpp/*.h*" "cpp/*.c*")
add_executable({0} ${{source_files}})
target_include_directories({0} PUBLIC "${{GLM_INCLUDE_DIR}}")
)^^^";
							cmake_lists << std::format(cmake_content, path.filename().stem().string());
							cmake_lists.close();

							auto shaders = ser_ui->var_addr<std::vector<std::string>>(sel_ref_obj, "shaders"_h);

						}
					}
				}
			}
		}
		else
			ImGui::Text("Multiple Files Selected");
	}
		break;
	}

	selection_changed = false;
}
                                