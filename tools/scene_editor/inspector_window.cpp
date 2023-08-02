#include "app.h"
#include "selection.h"
#include "history.h"
#include "inspector_window.h"
#include "scene_window.h"
#include "project_window.h"

#include <flame/foundation/typeinfo_serialize.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/material.h>
#include <flame/graphics/model.h>
#include <flame/graphics/animation.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/debug.h>
#include <flame/universe/timeline.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/armature.h>
#include <flame/universe/components/terrain.h>
#include <flame/universe/components/volume.h>
#include <flame/universe/components/particle_system.h>

InspectorWindow inspector_window;
static uint selection_changed_frame = 0;

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
		resize_vector(&vec, type, size);
	}

	void assign(void* _dst, void* _src)
	{
		if (!_dst)
			_dst = &v;
		if (!_src)
			_src = &v;
		copy_vector(_dst, _src, type);
	}
};

std::unordered_map<const void*, StagingVector> staging_vectors;

StagingVector& get_staging_vector(TypeInfo* type, void* vec)
{
	auto it = staging_vectors.find(vec);
	if (it != staging_vectors.end())
		return it->second;
	auto& ret = staging_vectors.emplace(vec, type).first->second;
	ret.assign(nullptr, vec);
	return ret;
}

struct EditingObjects
{
	int type;
	uint type2;
	void* objs;
	int num;
	std::unordered_map<const void*, uint>* sync_states;

	EditingObjects()
	{
		type = -1;
		num = 0;
	}

	EditingObjects(int type, uint type2, void* objs, int num, 
		std::unordered_map<const void*, uint>* sync_states = nullptr) :
		type(type),
		type2(type2),
		objs(objs),
		num(num),
		sync_states(sync_states)
	{
	}
};

std::stack<EditingObjects> editing_objects;

std::vector<std::string> before_editing_values;

void add_modify_history(uint attr_hash, const std::string& new_value)
{
	auto& eos = editing_objects.top();
	if (eos.num == 0)
		return;
	switch (eos.type)
	{
	case 0:
	{
		auto h = new AssetModifyHistory(*(std::filesystem::path*)eos.objs, eos.type2, attr_hash, before_editing_values[0], new_value);
		add_history(h);
		auto ui = find_udt(h->asset_type);
		auto attr = ui->find_attribute(h->attr_hash);
		app.last_status = std::format("Modified Asset: {}, ({}), {}: {} -> {}", h->path.string(), ui->name, attr->name, h->old_value, h->new_value);
	}
		break;
	case 1:
	{
		std::vector<GUID> ids(eos.num);
		for (auto i = 0; i < eos.num; i++)
			ids[i] = ((EntityPtr*)eos.objs)[i]->instance_id;
		auto h = new EntityModifyHistory(ids, eos.type2, attr_hash, before_editing_values, { new_value });
		add_history(h);
		auto ui = h->comp_type ? find_udt(h->comp_type) : TypeInfo::get<Entity>()->retrive_ui();
		auto attr = ui->find_attribute(h->attr_hash);
		if (h->ids.size() == 1)
		{
			auto e = ((EntityPtr*)eos.objs)[0];
			if (h->comp_type)
				app.last_status = std::format("Modified Component: {}, ({}), {}: {} -> {}", e->name, ui->name, attr->name, h->old_values[0], h->new_values[0]);
			else
				app.last_status = std::format("Modified Entity: {}, {}: {} -> {}", e->name, attr->name, h->old_values[0], h->new_values[0]);
		}
		else
		{
			if (h->comp_type)
				app.last_status = std::format("Modified {} Components: ({}), {}: {} -> {}", (int)h->ids.size(), ui->name, attr->name, h->old_values[0], h->new_values[0]);
			else
				app.last_status = std::format("Modified {} Entities: {}: * -> {}", (int)h->ids.size(), attr->name, h->new_values[0]);
		}
	}
		break;
	case 2:
	{
		auto h = new PrefabModifyHistory(*(std::filesystem::path*)eos.objs, eos.type2, attr_hash, before_editing_values[0], new_value);
		add_history(h);
		auto ui = h->comp_type ? find_udt(h->comp_type) : TypeInfo::get<Entity>()->retrive_ui();
		auto attr = ui->find_attribute(h->attr_hash);
		if (h->comp_type)
			app.last_status = std::format("Modified Prefab Component: {}, ({}), {}: {} -> {}", h->path.string(), ui->name, attr->name, h->old_value, h->new_value);
		else
			app.last_status = std::format("Modified Prefab Entity: {}, {}: {} -> {}", h->path.string(), attr->name, h->old_value, h->new_value);
	}
		break;
	}
}

std::pair<uint, uint> manipulate_udt(const UdtInfo& ui, voidptr* objs, uint num = 1, const std::vector<uint> excludes = {}, const std::function<void(uint)>&cb = {});

int manipulate_variable(TypeInfo* type, const std::string& name, uint name_hash, int offset, const FunctionInfo* getter, const FunctionInfo* setter, const std::string& default_value, 
	voidptr* objs, uint num, const void* id, bool hide_name = false)
{
	auto display_name = get_display_name(name);
	if (hide_name)
		display_name = "##" + display_name;
	auto changed = 0;
	bool just_exit_editing;
	auto direct_io = !getter && !setter;
	auto& eos = editing_objects.top();
	char same[4];
	if (num > 1)
	{
		auto v = eos.sync_states->at(id);
		memcpy(same, &v, sizeof(v));
	}
	else
	{
		same[0] = 1; same[1] = 1; same[2] = 1; same[3] = 1;
	}

	auto enable_record = false;
	if (eos.type == 1 && eos.num > 0)
	{
		if (app.timeline_recording)
		{
			enable_record = true;
			for (auto i = 0; i < eos.num; i++)
			{
				auto e = ((EntityPtr*)eos.objs)[i];
				if (e != app.e_timeline_host && !is_ancestor(app.e_timeline_host, e))
				{
					enable_record = false;
					break;
				}
			}
		}
	}

	auto get_keyframe_adress = [](EntityPtr t, EntityPtr e, uint comp_hash, const std::string& attr_name, int vector_component_index = -1)->std::string {
		std::string ret;
		if (t != e)
		{
			while (e->parent != t)
			{
				if (!ret.empty())
					ret = '.' + ret;
				ret = e->name + ret;
				e = e->parent;
			}
		}
		if (comp_hash)
		{
			if (!ret.empty())
				ret += '.';
			ret += find_udt(comp_hash)->name;
		}
		if (!ret.empty())
			ret += '|';
		ret += attr_name;
		static const char vector_component_names[] = { 'x', 'y', 'z', 'w' };
		if (vector_component_index != -1)
		{
			ret += '.';
			ret += vector_component_names[vector_component_index];
		}
		return ret;
	};

	auto input_int_n = [&](uint n, int* data) {
		auto ret = false;

		auto inner_spaceing = ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::BeginGroup();
		ImGui::PushID(name.c_str());
		ImGui::PushMultiItemsWidths(n, ImGui::CalcItemWidth());
		for (int i = 0; i < n; i++)
		{
			ImGui::PushID(i);
			if (i > 0)
				ImGui::SameLine(0.f, inner_spaceing);
			auto changed = ImGui::InputScalar("", ImGuiDataType_S32, &data[i], nullptr, nullptr, same[i] ? "%d" : "-", 0);
			if (enable_record)
			{
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::FontAtlas::icon_s("diamond"_h).c_str()))
				{
					for (auto j = 0; j < eos.num; j++)
					{
						auto address = get_keyframe_adress(app.e_timeline_host, ((EntityPtr*)eos.objs)[j], eos.type2, name, i);
						if (auto kf = app.get_keyframe(address, true); kf)
							kf->value = str(data[i]);
					}
				}
			}
			if (changed)
				same[i] = 0;
			ret |= changed;
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		ImGui::PopID();

		ImGui::SameLine(0.f, inner_spaceing);
		ImGui::TextEx(display_name.c_str());

		ImGui::EndGroup();
		return ret;
	};

	auto input_float_n = [&](uint n, float* data) {
		auto ret = 0;

		auto inner_spaceing = ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::BeginGroup();
		ImGui::PushID(name.c_str());
		ImGui::PushMultiItemsWidths(n, ImGui::CalcItemWidth());
		for (int i = 0; i < n; i++)
		{
			ImGui::PushID(i);
			if (i > 0)
				ImGui::SameLine(0.f, inner_spaceing);
			auto changed = ImGui::DragScalar("", ImGuiDataType_Float, &data[i], 0.1f, nullptr, nullptr, same[i] ? "%.3f" : "-", 0);
			if (enable_record)
			{
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::FontAtlas::icon_s("diamond"_h).c_str()))
				{
					for (auto j = 0; j < eos.num; j++)
					{
						auto address = get_keyframe_adress(app.e_timeline_host, ((EntityPtr*)eos.objs)[j], eos.type2, name, i);
						if (auto kf = app.get_keyframe(address, true); kf)
							kf->value = str(data[i]);
					}
				}
			}
			if (changed)
			{
				same[i] = 0;
				ret = 1;
				if (!ImGui::TempInputIsActive(ImGui::GetItemID()))
					ret = 2;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		ImGui::PopID();

		ImGui::SameLine(0.f, inner_spaceing);
		ImGui::TextEx(display_name.c_str());

		ImGui::EndGroup();
		return ret;
	};

	static TypeInfo* copied_type = nullptr;
	auto context_menu = [&](void* data) {
		if (!name_hash || hide_name)
			return;
		ImGui::SameLine();
		if (ImGui::Button("..."))
			ImGui::OpenPopup("context_menu");
		if (ImGui::BeginPopup("context_menu"))
		{
			if (ImGui::MenuItem("Reset Value"))
			{
				auto dv = !default_value.empty() ? default_value : type->serialize(nullptr);
				if (type->serialize(data) != dv)
				{
					type->unserialize(dv, data);
					changed = 3;
				}

			}
			if (ImGui::MenuItem("Copy Value"))
			{
				if (num == 1)
				{
					copied_type = type;
					set_clipboard(s2w(type->serialize(data)));
				}
			}
			if (ImGui::MenuItem("Paste Value"))
			{
				if (copied_type == type)
				{
					if (auto copied_value = w2s(get_clipboard()); type->serialize(data) != copied_value)
					{
						type->unserialize(copied_value, data);
						changed = 3;
					}
				}
			}
			ImGui::EndPopup();
		}
	};

	ImGui::PushID(id);
	switch (type->tag)
	{
	case TagE:
	{
		auto ti = (TypeInfo_Enum*)type;
		auto ei = ti->ei;
		auto data = type->get_value(objs[0], offset, getter, getter);
		int value = *(int*)data;
		if (!ei->is_flags)
		{
			value = clamp(value, 0, (int)ei->items.size() - 1);
			if (ImGui::BeginCombo(display_name.c_str(), same[0] ? ei->items[value].name.c_str() : "-"))
			{
				for (auto& ii : ei->items)
				{
					if (ImGui::Selectable(ii.name.c_str()))
					{
						if (!same[0] || value != ii.value)
						{
							value = ii.value;
							changed = true;
						}
					}
				}
				ImGui::EndCombo();
			}
			context_menu(data);
		}
		else
		{
			std::string str_value;
			if (same[0])
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
			context_menu(data);
		}
		if (changed)
		{
			before_editing_values.resize(num);
			for (auto i = 0; i < num; i++)
				before_editing_values[i] = str(*(int*)type->get_value(objs[i], offset, getter));
			for (auto i = 0; i < num; i++)
				type->set_value(objs[i], offset, setter, &value);
			add_modify_history(name_hash, str(value));
			if (num > 1)
				eos.sync_states->at(id) = 1;

			changed = 2;
		}
	}
		break;
	case TagD:
	{
		auto ti = (TypeInfo_Data*)type;
		auto vec_size = ti->vec_size;
		auto data = type->get_value(objs[0], offset, getter, !direct_io);
		switch (ti->data_type)
		{
		case DataBool:
		{
			auto value = same[0] ? *(bool*)data : true;
			if (!same[0])
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
			changed = ImGui::Checkbox(display_name.c_str(), &value);
			if (!same[0])
				ImGui::PopItemFlag();
			context_menu(data);
			if (changed)
			{
				before_editing_values.resize(num);
				for (auto i = 0; i < num; i++)
					before_editing_values[i] = str(*(bool*)type->get_value(objs[i], offset, getter));
				for (auto i = 0; i < num; i++)
					type->set_value(objs[i], offset, setter, &value);
				add_modify_history(name_hash, str(value));
				if (num > 1)
					eos.sync_states->at(id) = 1;
			}
			data = nullptr;
			if (changed)
				changed = 2;
		}
			break;
		case DataInt:
			changed = input_int_n(ti->vec_size, (int*)data);
			if (ImGui::IsItemActivated())
			{
				before_editing_values.resize(num);
				before_editing_values[0] = str(ti->vec_size, (int*)data);
				for (auto i = 1; i < num; i++)
					before_editing_values[i] = str(ti->vec_size, (int*)type->get_value(objs[i], offset, getter));
			}
			just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
			if (changed)
			{
				if (just_exit_editing && !direct_io)
					type->set_value(objs[0], offset, setter, data);
				if (direct_io || just_exit_editing)
				{
					for (auto i = 1; i < num; i++)
						type->set_value(objs[i], offset, setter, data);
				}
				if (num > 1)
					eos.sync_states->at(id) = 1;
			}
			context_menu(data);
			if (just_exit_editing)
			{
				auto new_value = str(ti->vec_size, (int*)data);
				add_modify_history(name_hash, new_value);
				if (enable_record)
				{
					auto sp = SUS::split(new_value, ',');
					for (auto i = 0; i < eos.num; i++)
					{
						auto sp2 = SUS::split(before_editing_values[i], ',');
						for (auto j = 0; j < sp.size(); j++)
						{
							if (sp[j] != sp2[j])
							{
								auto address = get_keyframe_adress(app.e_timeline_host, ((EntityPtr*)eos.objs)[i], eos.type2, name, j);
								if (auto kf = app.get_keyframe(address, false); kf)
									kf->value = sp[j];
							}
						}
					}
				}
			}
			changed = just_exit_editing ? 2 : changed > 0;
			break;
		case DataFloat:
			changed = input_float_n(ti->vec_size, (float*)data);
			if (ImGui::IsItemActivated())
			{
				before_editing_values.resize(num);
				before_editing_values[0] = str(ti->vec_size, (float*)data);
				for (auto i = 1; i < num; i++)
					before_editing_values[i] = str(ti->vec_size, (float*)type->get_value(objs[i], offset, getter));
			}
			just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
			context_menu(data);
			if (changed)
			{
				if ((changed > 1 || just_exit_editing) && !direct_io)
					type->set_value(objs[0], offset, setter, data);
				if (direct_io || (changed > 1 || just_exit_editing))
				{
					for (auto i = 1; i < num; i++)
						type->set_value(objs[i], offset, setter, data);
				}
				if (num > 1)
					eos.sync_states->at(id) = 1;
			}
			if (just_exit_editing)
			{
				auto new_value = str(ti->vec_size, (float*)data);
				add_modify_history(name_hash, new_value);
				if (enable_record)
				{
					auto sp = SUS::split(new_value, ',');
					for (auto i = 0; i < eos.num; i++)
					{
						auto sp2 = SUS::split(before_editing_values[i], ',');
						for (auto j = 0; j < sp.size(); j++)
						{
							if (sp[j] != sp2[j])
							{
								auto address = get_keyframe_adress(app.e_timeline_host, ((EntityPtr*)eos.objs)[i], eos.type2, name, j);
								if (auto kf = app.get_keyframe(address, false); kf)
									kf->value = sp[j];
							}
						}
					}
				}
			}
			changed = just_exit_editing ? 2 : changed > 0;
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
				if (ImGui::IsItemActivated())
				{
					before_editing_values.resize(num);
					before_editing_values[0] = str(*(cvec4*)data);
					for (auto i = 1; i < num; i++)
						before_editing_values[i] = str(*(cvec4*)type->get_value(objs[i], offset, getter));
				}
				just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
				context_menu(data);
				if (changed)
				{
					if (!direct_io)
						type->set_value(objs[0], offset, setter, data);
					if (direct_io)
					{
						for (auto i = 1; i < num; i++)
							type->set_value(objs[i], offset, setter, data);
					}
					if (num > 1)
						eos.sync_states->at(id) = 1;
				}
				if (just_exit_editing)
					add_modify_history(name_hash, str(*(cvec4*)data));
				if (changed)
					changed = 2;
			}
				break;
			}
			break;
		case DataString:
		{
			if (same[0])
				changed = ImGui::InputText(display_name.c_str(), (std::string*)data);
			else
			{
				std::string s = "-";
				changed = ImGui::InputText(display_name.c_str(), &s);
				if (changed)
					*(std::string*)data = s;
			}
			if (ImGui::IsItemActivated())
			{
				before_editing_values.resize(num);
				before_editing_values[0] = *(std::string*)data;
				for (auto i = 1; i < num; i++)
					before_editing_values[i] = *(std::string*)type->get_value(objs[i], offset, getter);
			}
			just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
			context_menu(data);
			if (changed)
			{
				if (just_exit_editing && !direct_io)
					type->set_value(objs[0], offset, setter, data);
				if (direct_io || just_exit_editing)
				{
					for (auto i = 1; i < num; i++)
						type->set_value(objs[i], offset, setter, data);
				}
				if (num > 1)
					eos.sync_states->at(id) = 1;
			}
			if (just_exit_editing)
				add_modify_history(name_hash, *(std::string*)data);
			changed = just_exit_editing ? 2 : changed > 0;
		}
			break;
		case DataWString:
		{
			if (same[0])
			{
				auto s = w2s(*(std::wstring*)data);
				changed = ImGui::InputText(display_name.c_str(), &s);
				if (changed)
					*(std::wstring*)data = s2w(s);
			}
			else
			{
				std::string s = "-";
				changed = ImGui::InputText(display_name.c_str(), &s);
				if (changed)
					*(std::wstring*)data = s2w(s);
			}
			if (ImGui::IsItemActivated())
			{
				before_editing_values.resize(num);
				before_editing_values[0] = w2s(*(std::wstring*)data);
				for (auto i = 1; i < num; i++)
					before_editing_values[i] = w2s(*(std::wstring*)type->get_value(objs[i], offset, getter));
			}
			just_exit_editing = ImGui::IsItemDeactivatedAfterEdit();
			context_menu(data);
			if (changed)
			{
				if (just_exit_editing && !direct_io)
					type->set_value(objs[0], offset, setter, data);
				if (direct_io || just_exit_editing)
				{
					for (auto i = 1; i < num; i++)
						type->set_value(objs[i], offset, setter, data);
				}
				if (num > 1)
					eos.sync_states->at(id) = 1;
			}
			if (just_exit_editing)
				add_modify_history(name_hash, w2s(*(std::wstring*)data));
			changed = just_exit_editing ? 2 : changed > 0;
		}
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
					before_editing_values.resize(num);
					before_editing_values[0] = path.string();
					for (auto i = 1; i < num; i++)
						before_editing_values[i] = (*(std::filesystem::path*)type->get_value(objs[i], offset, getter)).string();

					path = Path::reverse(std::wstring((wchar_t*)payload->Data));
					changed = true;
					add_modify_history(name_hash, path.string());
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			if (ImGui::Button(graphics::FontAtlas::icon_s("location-crosshairs"_h).c_str()))
				project_window.ping(Path::get(path));
			ImGui::SameLine();
			if (ImGui::Button(graphics::FontAtlas::icon_s("xmark"_h).c_str()))
			{
				before_editing_values.resize(num);
				before_editing_values[0] = path.string();
				for (auto i = 1; i < num; i++)
					before_editing_values[i] = (*(std::filesystem::path*)type->get_value(objs[i], offset, getter)).string();

				path = L"";
				changed = true;
				add_modify_history(name_hash, path.string());
			}
			context_menu(data);
			if (changed)
			{
				if (!direct_io)
					type->set_value(objs[0], offset, setter, data);
				for (auto i = 1; i < num; i++)
					type->set_value(objs[i], offset, setter, data);
				if (num > 1)
					eos.sync_states->at(id) = 1;

				changed = 2;
			}
		}
			break;
		case DataGUID:
		{
			GUID& guid = *(GUID*)data;
			auto s = guid.to_string();
			ImGui::InputText(display_name.c_str(), s.data(), ImGuiInputTextFlags_ReadOnly);
			if (ImGui::BeginDragDropTarget())
			{
				if (auto payload = ImGui::AcceptDragDropPayload("Entity"); payload)
				{
					before_editing_values.resize(num);
					before_editing_values[0] = guid.to_string();
					for (auto i = 1; i < num; i++)
						before_editing_values[i] = (*(GUID*)type->get_value(objs[i], offset, getter)).to_string();

					auto entity = *(EntityPtr*)payload->Data;

					guid = entity->file_id;
					changed = true;
					add_modify_history(name_hash, guid.to_string());
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			if (ImGui::Button(graphics::FontAtlas::icon_s("location-crosshairs"_h).c_str()))
			{
				add_event([guid]() {
					auto e = app.e_prefab ? app.e_prefab->find_with_file_id(guid) : nullptr;
					selection.select(e, "app"_h);
					return false;
				});
			}
			ImGui::SameLine();
			if (ImGui::Button(graphics::FontAtlas::icon_s("xmark"_h).c_str()))
			{
				before_editing_values.resize(num);
				before_editing_values[0] = guid.to_string();
				for (auto i = 1; i < num; i++)
					before_editing_values[i] = (*(GUID*)type->get_value(objs[i], offset, getter)).to_string();

				guid.reset();
				changed = true;
				add_modify_history(name_hash, guid.to_string());
			}
			context_menu(data);
			if (changed)
			{
				if (!direct_io)
					type->set_value(objs[0], offset, setter, data);
				for (auto i = 1; i < num; i++)
					type->set_value(objs[i], offset, setter, data);
				if (num > 1)
					eos.sync_states->at(id) = 1;

				changed = 2;
			}
		}
			break;
		}
	}
		break;
	case TagU:
		if (num == 1 && ImGui::TreeNode(display_name.c_str()))
		{
			auto ti = (TypeInfo_Udt*)type;
			auto ui = ti->ui;
			editing_objects.push(EditingObjects());

			voidptr ptr = (char*)objs[0] + offset;
			if (manipulate_udt(*ui, &ptr).first)
				changed = true;

			editing_objects.pop();
			ImGui::TreePop();
		}
		break;
	case TagO:
		if (num == 1 && ImGui::TreeNode(display_name.c_str()))
		{
			editing_objects.push(EditingObjects());

			auto& vo = *(VirtualUdt<int>*)((char*)objs[0] + offset);
			static std::vector<UdtInfo*> available_types;
			ImGui::Text("[%s]", vo.type ? vo.type->name.c_str() : "");
			ImGui::SameLine();
			if (ImGui::Button("T"))
			{
				ImGui::OpenPopup("select_type");
				available_types.clear();
				auto base_name = type->name;
				for (auto& [_, ui] : tidb.udts)
				{
					if (ui.base_class_name == base_name)
						available_types.push_back(&ui);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(graphics::FontAtlas::icon_s("xmark"_h).c_str()))
			{
				if (vo.data)
					vo.destroy();
				vo.type = nullptr;
				changed = true;
			}
			if (ImGui::BeginPopup("select_type"))
			{
				for (auto ui : available_types)
				{
					if (ImGui::Selectable(ui->name.c_str()))
					{
						if (vo.data)
							vo.destroy();
						vo.type = TypeInfo::get(TagU, ui->name, *ui->db);
						vo.create();
						changed = true;
					}
				}
				ImGui::EndPopup();
			}
			if (vo.data)
			{
				voidptr ptr = vo.data;
				if (manipulate_udt(*vo.type->retrive_ui(), &ptr).first)
					changed = true;
			}

			editing_objects.pop();
			ImGui::TreePop();
		}
		break;
	case TagVD:
		if (num == 1 && ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)objs[0] + offset;
			auto ti = ((TypeInfo_VectorOfData*)type)->ti;
			auto& sv = get_staging_vector(ti, pv);
			auto set_sv = [&]() {
				if (!setter)
					sv.assign(pv, nullptr);
				else
					type->set_value(objs[0], offset, setter, &sv.v);
			};
			int n = sv.count();
			auto size_changed = ImGui::InputInt("size", &n, 1, 1);
			context_menu(pv);
			ImGui::Separator();
			if (size_changed)
			{
				sv.resize(nullptr, n);
				set_sv();

				changed = 2;
			}
			else
				n = sv.count();
			if (!changed)
			{
				editing_objects.push(EditingObjects());
				for (auto i = 0; i < n; i++)
				{
					ImGui::PushID(i);
					auto ptr = sv.v.data();
					if (manipulate_variable(ti, str(i), 0, i * ti->size, nullptr, nullptr, "", (voidptr*)&ptr, 1, id) > 1)
						changed = true;
					ImGui::PopID();
				}
				editing_objects.pop();
				if (changed)
				{
					set_sv();

					changed = 2;
				}
			}
			ImGui::TreePop();
		}
		break;
	case TagVU:
		if (num == 1 && ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)objs[0] + offset;
			auto ti = ((TypeInfo_VectorOfUdt*)type)->ti;
			auto& sv = get_staging_vector(ti, pv);
			auto set_sv = [&]() {
				if (!setter)
					sv.assign(pv, nullptr);
				else
					type->set_value(objs[0], offset, setter, &sv.v);
			};
			auto& ui = *ti->retrive_ui();
			int n = sv.count();
			auto size_changed = ImGui::InputInt("size", &n, 1, 1);
			context_menu(pv);
			ImGui::Separator();
			if (size_changed)
			{
				sv.resize(nullptr, n);
				set_sv();

				changed = 2;
			}
			else
				n = sv.count();
			if (!changed)
			{
				editing_objects.push(EditingObjects());
				for (auto i = 0; i < n; i++)
				{
					if (i > 0) ImGui::Separator();
					ImGui::PushID(i);
					voidptr obj = sv.v.data() + ui.size * i;
					if (manipulate_udt(ui, &obj).first)
						changed = true;
					ImGui::PopID();
				}
				editing_objects.pop();
				if (changed)
				{
					set_sv();

					changed = 2;
				}
			}
			ImGui::TreePop();
		}
		break;
	case TagVR:
		if (num == 1 && ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)objs[0] + offset;
			auto ti = ((TypeInfo_VectorOfPair*)type)->ti;
			auto& sv = get_staging_vector(ti, pv);
			auto set_sv = [&]() {
				if (!setter)
					sv.assign(pv, nullptr);
				else
					type->set_value(objs[0], offset, setter, &sv.v);
			};
			int n = sv.count();
			auto size_changed = ImGui::InputInt("size", &n, 1, 1);
			context_menu(pv);
			ImGui::Separator();
			if (size_changed)
			{
				sv.resize(nullptr, n);
				set_sv();

				changed = 2;
			}
			else
				n = sv.count();
			if (!changed)
			{
				editing_objects.push(EditingObjects());
				for (auto i = 0; i < n; i++)
				{
					if (i > 0) ImGui::Separator();
					ImGui::PushID(i);
					auto p = sv.v.data() + ti->size * i;
					auto ptr0 = ti->first(p);
					auto ptr1 = ti->second(p);
					if (manipulate_variable(ti->ti1, "First", 0, 0, nullptr, nullptr, "", (voidptr*)&ptr0, 1, id) > 1)
						changed = true;
					if (manipulate_variable(ti->ti2, "Second", 0, 0, nullptr, nullptr, "", (voidptr*)&ptr1, 1, id) > 1)
						changed = true;
					ImGui::PopID();
				}
				editing_objects.pop();
				if (changed)
				{
					set_sv();

					changed = 2;
				}
			}
			ImGui::TreePop();
		}
		break;
	case TagVT:
		if (num == 1 && ImGui::TreeNode(display_name.c_str()))
		{
			assert(!getter);
			auto pv = (char*)objs[0] + offset;
			auto ti = ((TypeInfo_VectorOfTuple*)type)->ti;
			auto& sv = get_staging_vector(ti, pv);
			auto set_sv = [&]() {
				if (!setter)
					sv.assign(pv, nullptr);
				else
					type->set_value(objs[0], offset, setter, &sv.v);
			};
			if (ImGui::IsItemDeactivated())
				sv.assign(nullptr, pv);
			int n = sv.count();
			auto size_changed = ImGui::InputInt("size", &n, 1, 1);
			context_menu(pv);
			ImGui::Separator();
			if (size_changed)
			{
				sv.resize(nullptr, n);
				set_sv();

				changed = 2;
			}
			else
				n = sv.count();
			if (!changed)
			{
				editing_objects.push(EditingObjects());
				for (auto i = 0; i < n; i++)
				{
					if (i > 0) ImGui::Separator();
					ImGui::PushID(i);
					auto p = sv.v.data() + ti->size * i;
					auto j = 0;
					for (auto& t : ti->tis)
					{
						auto ptr = p + t.second;
						if (manipulate_variable(t.first, "Item " + str(j), 0, 0, nullptr, nullptr, "", (voidptr*)&ptr, 1, id) > 1)
							changed = true;
						j++;
					}
					ImGui::PopID();
				}
				editing_objects.pop();
				if (changed)
				{
					set_sv();

					changed = 2;
				}
			}
			ImGui::TreePop();
		}
		break;
	}
	ImGui::PopID();

	return changed;
}

int manipulate_variable(const VariableInfo& v, voidptr* objs, uint num)
{
	return manipulate_variable(v.type, v.name, v.name_hash, v.offset, nullptr, nullptr, v.default_value, objs, num, &v);
}

int manipulate_attribute(const Attribute& a, voidptr* objs, uint num, bool hide_name = false)
{
	return manipulate_variable(a.type, a.name, a.name_hash, a.var_off(),
		a.getter_idx != -1 ? &a.ui->functions[a.getter_idx] : nullptr,
		a.setter_idx != -1 ? &a.ui->functions[a.setter_idx] : nullptr,
		a.default_value,
		objs, num, &a, hide_name);
}

std::pair<uint, uint> manipulate_udt(const UdtInfo& ui, voidptr* objs, uint num, const std::vector<uint> excludes, const std::function<void(uint)>& cb)
{
	uint ret_changed = 0;
	uint ret_changed_name = 0;

	if (ui.attributes.empty())
	{
		for (auto& v : ui.variables)
		{
			bool skip = false;
			if (!excludes.empty())
			{
				for (auto h : excludes)
				{
					if (h == v.name_hash)
					{
						skip = true;
						break;
					}
				}
			}
			if (skip)
				continue;
			auto changed = manipulate_variable(v, objs, num);
			ret_changed |= changed;
			if (changed)
				ret_changed_name = v.name_hash;
			if (cb)
				cb(v.name_hash);
		}
	}
	else
	{
		for (auto& a : ui.attributes)
		{
			bool skip = false;
			if (!excludes.empty())
			{
				for (auto h : excludes)
				{
					if (h == a.name_hash)
					{
						skip = true;
						break;
					}
				}
			}
			if (skip)
				continue;
			auto changed = manipulate_attribute(a, objs, num);
			ret_changed |= changed;
			if (changed)
				ret_changed_name = a.name_hash;
			if (cb)
				cb(a.name_hash);
		}
	}
	return std::make_pair(ret_changed, ret_changed_name);
}

static auto ui_entity = TypeInfo::get<Entity>()->retrive_ui();
static auto ui_component = TypeInfo::get<Component>()->retrive_ui();

InspectedEntities::~InspectedEntities()
{
	for (auto e : entities)
		e->message_listeners.remove("inspector"_h);
}

void InspectedEntities::refresh(const std::vector<EntityPtr>& _entities)
{
	for (auto e : entities)
		e->message_listeners.remove("inspector"_h);
	entities.clear();
	sync_states.clear();
	common_components.clear();
	prefab_path.clear();

	entities = _entities;
	if (entities.empty())
		return;
	for (auto e : entities)
	{
		e->message_listeners.add([this, e](uint hash, void*, void*) {
			if (hash == "destroyed"_h)
			{
				for (auto it = entities.begin(); it != entities.end();)
				{
					if (*it == e)
						it = entities.erase(it);
					else
						it++;
				}
				if (entities.empty() && inspector->inspected_type == Selection::tEntity)
				{
					inspector->locked = false;
					inspector->last_select_frame = 0;
				}
			}
		}, "inspector"_h);
	}

	auto entt0 = entities[0];
	auto process_attribute = [&](const Attribute& a, uint comp_hash) {
		void* obj0 = comp_hash == 0 ? entt0 : (void*)entt0->get_component_h(comp_hash);
		uint state = 1;

		if (a.type->tag == TagD)
		{
			auto var0 = a.type->create();
			a.type->copy(var0, a.get_value(obj0));
			auto ti = (TypeInfo_Data*)a.type;
			switch (ti->data_type)
			{
			case DataBool:
				for (auto i = 1; i < entities.size(); i++)
				{
					void* obj1 = comp_hash == 0 ? entities[i] : (void*)entities[i]->get_component_h(comp_hash);
					auto var1 = a.get_value(obj1);
					if (*(bool*)var0 != *(bool*)var1)
					{
						state = 0;
						break;
					}
				}
				break;
			case DataInt:
			case DataFloat:
				for (auto i = 1; i < entities.size(); i++)
				{
					void* obj1 = comp_hash == 0 ? entities[i] : (void*)entities[i]->get_component_h(comp_hash);
					auto var1 = a.get_value(obj1);
					for (auto y = 0; y < ti->vec_size; y++)
					{
						if (memcmp((char*)var0 + sizeof(float) * y, (char*)var1 + sizeof(float) * y, sizeof(float)) != 0)
							((char*)&state)[y] = 0;
						else
							((char*)&state)[y] = 1;
					}
				}
				break;
			default:
				for (auto i = 1; i < entities.size(); i++)
				{
					void* obj1 = comp_hash == 0 ? entities[i] : (void*)entities[i]->get_component_h(comp_hash);
					if (!a.type->compare(var0, a.get_value(obj1)))
					{
						state = 0;
						break;
					}
				}
			}
			a.type->destroy(var0);
		}

		sync_states[&a] = state;
	};

	if (entities.size() > 1)
	{
		for (auto& a : ui_entity->attributes)
			process_attribute(a, 0);
	}

	for (auto& comp : entt0->components)
	{
		auto hash = comp->type_hash;
		auto all_have = true;
		for (auto i = 1; i < entities.size(); i++)
		{
			if (!entities[i]->get_component_h(hash))
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
				cc.components[i] = entities[i]->get_component_h(hash);
		}
	}
	if (entities.size() > 1)
	{
		for (auto& cc : common_components)
		{
			auto comp0 = entt0->get_component_h(cc.type_hash);
			auto& ui = *find_udt(cc.type_hash);
			for (auto& a : ui_component->attributes)
				process_attribute(a, cc.type_hash);
			for (auto& a : ui.attributes)
				process_attribute(a, cc.type_hash);
		}
	}
}

std::pair<uint, uint> InspectedEntities::manipulate()
{
	uint ret_changed = 0;
	uint ret_changed_name = 0;
	auto get_changed = [&](const std::pair<uint, uint>& res) {
		ret_changed |= res.first;
		ret_changed_name = res.second;
	};
	auto get_changed2 = [&](uint changed, uint hash) {
		ret_changed |= changed;
		if (changed)
			ret_changed_name = hash;
	};

	auto entity = entities[0];

	if (prefab_path.empty())
		editing_objects.emplace(EditingObjects(1, 0, entities.data(), entities.size(), &sync_states));
	else
		editing_objects.emplace(EditingObjects(2, 0, &prefab_path, 1, nullptr));
	ImGui::PushID("flame::Entity"_h);
	{
		auto hash = "enable"_h;
		get_changed2(manipulate_attribute(*ui_entity->find_attribute(hash), (voidptr*)entities.data(), entities.size(), true), hash);
	}
	ImGui::SameLine();
	{
		auto hash = "name"_h;
		get_changed2(manipulate_attribute(*ui_entity->find_attribute(hash), (voidptr*)entities.data(), entities.size(), true), hash);
	}
	ImGui::SameLine();
	{
		ImGui::SetNextItemWidth(100.f);
		auto hash = "tag"_h;
		get_changed2(manipulate_attribute(*ui_entity->find_attribute(hash), (voidptr*)entities.data(), entities.size(), true), hash);
	}
	if (entities.size() == 1 && entity->prefab_instance)
	{
		auto ins = entity->prefab_instance.get();
		auto& path = ins->filename;
		auto str = path.string();
		ImGui::InputText("Prefab", str.data(), ImGuiInputTextFlags_ReadOnly);
		ImGui::SameLine();
		if (ImGui::Button(graphics::FontAtlas::icon_s("location-crosshairs"_h).c_str()))
			project_window.ping(Path::get(path));
		ImGui::SameLine();
		ImGui::Button(("Modifications " + graphics::FontAtlas::icon_s("angle-down"_h)).c_str());
		if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
		{
			for (auto& m : ins->modifications)
				ImGui::Text(m.c_str());
			if (ins->modifications.empty())
			{
				if (ImGui::Button("Seize Modifications and Apply"))
				{
					std::string str;
					auto& root_ins_mods = get_root_prefab_instance(entity)->modifications;
					std::vector<int> seize_indices;
					uint off = 0;
					for (auto i = 0; i < root_ins_mods.size(); i++)
					{
						auto sp = SUS::split(root_ins_mods[i], '|');
						GUID guid;
						guid.from_string(std::string(sp.front()));
						if (entity->find_with_file_id(guid))
						{
							str += root_ins_mods[i] + "\n";
							seize_indices.push_back(i - off);
							off++;
						}
					}
					auto prompt = std::format("This action cannot be redo\n"
						"This action will first filter modificaions that their targets are in this prefab, \n"
						"and then seizes them from the prefab root, finally apply them to this prefab\n\n"
						"{} modifications will be seized and apply:\n{}", (int)seize_indices.size(), str);

					ImGui::OpenYesNoDialog("Are you sure to seize modificaitons and apply?", prompt, [entity, ins, &root_ins_mods, seize_indices](bool yes) {
						if (yes && !seize_indices.empty())
						{
							for (auto i : seize_indices)
								root_ins_mods.erase(root_ins_mods.begin() + i);
							entity->save(Path::get(ins->filename), true);
							ins->modifications.clear();
						}
						});
				}
			}
			else
			{
				if (ImGui::Button("Apply"))
				{
					ImGui::OpenYesNoDialog("Are you sure to apply all modifications?", "This action cannot be redo", [entity, ins](bool yes) {
						if (yes && !ins->modifications.empty())
						{
							entity->save(Path::get(ins->filename), true);
							ins->modifications.clear();
						}
						});
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Discard"))
			{
				ImGui::OpenYesNoDialog("Are you sure to revert all modifications?", "This action cannot be redo", [this, entity, ins](bool yes) {
					if (yes)
					{
						if (!ins->modifications.empty())
						{
							add_event([this, ins, entity]() {
								empty_entity(entity);
								entity->load(ins->filename);
								auto es = entities;
								refresh(es);
								return false;
								});
							ins->modifications.clear();
						}
					}
					});
			}
			ImGui::EndPopup();
		}
	}
	ImGui::PopID();
	editing_objects.pop();

	if (ret_changed_name != 0)
	{
		auto& str = ui_entity->find_attribute(ret_changed_name)->name;
		for (auto e : entities)
		{
			if (auto ins = get_root_prefab_instance(e); ins)
				ins->mark_modification(e->file_id.to_string() + '|' + str);
		}
	}

	bool exit_editing = false;
	for (auto& cc : common_components)
	{
		if (prefab_path.empty())
			editing_objects.emplace(EditingObjects(1, th<Component>(), entities.data(), entities.size(), &sync_states));
		else
			editing_objects.emplace(EditingObjects(2, th<Component>(), &prefab_path, 1, nullptr));
		ImGui::PushID(cc.type_hash);
		auto open = ImGui::CollapsingHeader("", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
		ImGui::SameLine();
		{
			auto hash = "enable"_h;
			get_changed2(manipulate_attribute(*ui_component->find_attribute(hash), (voidptr*)cc.components.data(), cc.components.size(), true), hash);
		}
		editing_objects.pop();

		if (prefab_path.empty())
			editing_objects.emplace(EditingObjects(1, cc.type_hash, entities.data(), entities.size(), &sync_states));
		else
			editing_objects.emplace(EditingObjects(2, cc.type_hash, &prefab_path, 1, nullptr));
		auto& ui = *find_udt(cc.type_hash);
		ImGui::SameLine();
		ImGui::TextUnformatted(ui.name.c_str());
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("..."))
			ImGui::OpenPopup("component_menu");
		if (ImGui::BeginPopup("component_menu"))
		{
			if (ImGui::Selectable("Move Up"))
			{
				auto ok = true;
				for (auto e : entities)
				{
					if (auto ins = get_root_prefab_instance(e); ins)
					{
						ok = false;
						if (auto comp_idx = e->find_component_i(cc.type_hash); comp_idx > 0)
						{
							if (ins->find_modification(e->file_id.to_string() + '|' + find_udt(cc.type_hash)->name + "|add") != -1 &&
								ins->find_modification(e->file_id.to_string() + '|' + find_udt(e->components[comp_idx - 1]->type_hash)->name + "|add") != -1)
								ok = true;
						}
						if (!ok)
						{
							open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
							break;
						}
					}
				}
				if (ok)
				{
					auto changed = false;
					for (auto e : entities)
					{
						if (auto comp_idx = e->find_component_i(cc.type_hash); comp_idx > 0)
							changed |= e->reposition_component(comp_idx, comp_idx - 1);
					}
					if (changed)
					{
						ret_changed |= 2;
						auto es = entities;
						refresh(es);
						exit_editing = true;
					}
				}
			}
			if (ImGui::Selectable("Move Down"))
			{
				auto ok = true;
				for (auto e : entities)
				{
					if (auto ins = get_root_prefab_instance(e); ins)
					{
						ok = false;
						if (auto comp_idx = e->find_component_i(cc.type_hash); comp_idx < e->components.size() - 1)
						{
							if (ins->find_modification(e->file_id.to_string() + '|' + find_udt(cc.type_hash)->name + "|add") != -1 &&
								ins->find_modification(e->file_id.to_string() + '|' + find_udt(e->components[comp_idx + 1]->type_hash)->name + "|add") != -1)
								ok = true;
						}
						if (!ok)
						{
							open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
							break;
						}
					}
				}
				if (ok)
				{
					auto changed = false;
					for (auto e : entities)
					{
						if (auto comp_idx = e->find_component_i(cc.type_hash); comp_idx < e->components.size() - 1)
							changed |= e->reposition_component(comp_idx, comp_idx + 1);
					}
					if (changed)
					{
						ret_changed |= 2;
						auto es = entities;
						refresh(es);
						exit_editing = true;
					}
				}
			}
			if (ImGui::Selectable("Remove"))
			{
				auto changed = false;
				for (auto e : entities)
				{
					if (e->remove_component_h(cc.type_hash))
					{
						if (auto ins = get_root_prefab_instance(e); ins)
						{
							auto idx = ins->find_modification(e->file_id.to_string() + '|' + ui.name + "|add");
							if (idx == -1)
								ins->mark_modification(e->file_id.to_string() + '|' + ui.name + "|remove");
							else
							{
								auto target_string = e->file_id.to_string() + '|' + ui.name;
								for (auto it = ins->modifications.begin(); it != ins->modifications.end();)
								{
									if (it->starts_with(target_string))
										it = ins->modifications.erase(it);
									else
										it++;

								}
							}
						}
						changed = true;
					}
					if (changed)
					{
						ret_changed |= 2;
						auto es = entities;
						refresh(es);
						exit_editing = true;
					}
				}
			}
			static uint copied_component = 0;
			static std::vector<std::pair<uint, std::string>> copied_values;
			if (ImGui::Selectable("Reset Values"))
			{
				auto changed = false;
				for (auto& a : ui.attributes)
				{
					auto different = false;
					for (auto obj : cc.components)
					{
						if (a.serialize(obj) != a.default_value)
						{
							different = true;
							break;
						}
					}

					if (different)
					{
						before_editing_values.resize(cc.components.size());
						for (auto i = 0; i < cc.components.size(); i++)
							before_editing_values[i] = a.serialize(cc.components[i]);

						for (auto obj : cc.components)
						{
							a.unserialize(obj, a.default_value);
							changed = true;
						}
						add_modify_history(a.name_hash, a.default_value);
					}
				}
				if (changed)
					ret_changed |= 3;
			}
			if (cc.components.size() == 1)
			{
				if (ImGui::Selectable("Copy Values"))
				{
					copied_component = cc.type_hash;
					copied_values.clear();
					auto obj = cc.components[0];
					for (auto& a : ui.attributes)
					{
						if (a.type->tag >= TagP_Beg && a.type->tag <= TagP_End)
							continue;
						auto& v = copied_values.emplace_back();
						v.first = a.name_hash;
						v.second = a.serialize(obj);
					}
				}
			}
			if (ImGui::Selectable("Paste Values"))
			{
				if (cc.type_hash == copied_component)
				{
					auto changed = false;
					for (auto& v : copied_values)
					{
						if (auto a = ui.find_attribute(v.first); a)
						{
							auto different = false;
							for (auto obj : cc.components)
							{
								if (a->serialize(obj) != v.second)
								{
									different = true;
									break;
								}
							}

							if (different)
							{
								before_editing_values.resize(cc.components.size());
								for (auto i = 0; i < cc.components.size(); i++)
									before_editing_values[i] = a->serialize(cc.components[i]);

								for (auto obj : cc.components)
								{
									a->unserialize(obj, v.second);
									changed = true;
								}
								add_modify_history(a->name_hash, v.second);
							}
						}
					}
					if (changed)
						ret_changed |= 3;
				}
			}
			ImGui::EndPopup();
		}

		if (open && !exit_editing)
		{
			static bool open_select_standard_model = false;
			static bool open_select_hash = false;
			static std::vector<std::string> hash_candidates;
			static const Attribute* op_attr;
			get_changed(manipulate_udt(ui, (voidptr*)cc.components.data(), cc.components.size(), {}, [&ui, &cc](uint name) {
				ImGui::PushID(name);
				if (name == "mesh_name"_h)
				{
					ImGui::SameLine();
					if (ImGui::Button("S"))
					{
						open_select_standard_model = true;
						op_attr = ui.find_attribute(name);
					}
				}
				else if (name == "material_name"_h)
				{
					ImGui::SameLine();
					if (ImGui::Button("D"))
					{
						auto path = std::filesystem::path(L"default");
						ui.find_function("set_material_name"_h)->call<void, void*>(cc.components[0], &path);
					}

					auto& name = *(std::filesystem::path*)ui.find_attribute("material_name"_h)->get_value(cc.components[0]);
					if (!name.empty() && name != L"default" && !name.native().starts_with(L"0x"))
					{
						if (ImGui::TreeNode("##embed"))
						{
							// the material is loaded and registered to renderer
							if (auto material = graphics::Material::get(name); material)
							{
								editing_objects.emplace(EditingObjects(0, th<graphics::Material>(), &name, 1));
								auto changed = manipulate_udt(*TypeInfo::get<graphics::Material>()->retrive_ui(), (voidptr*)&material, 1).first;
								editing_objects.pop();
								graphics::Material::release(material);
								if (changed >= 2)
								{
									auto path = Path::get(name);
									material->save(path);
									auto asset = AssetManagemant::find(path);
									if (asset)
										asset->lwt = std::filesystem::last_write_time(path);
								}
							}
							ImGui::TreePop();
						}
					}
				}
				else
				{
					auto& a = *ui.find_attribute(name);
					if (a.var_idx != -1)
					{
						std::string meta;
						if (ui.variables[a.var_idx].metas.get("hash"_h, &meta))
						{
							ImGui::SameLine();
							if (ImGui::Button("S"))
							{
								open_select_hash = true;
								hash_candidates = SUS::to_string_vector(SUS::split(meta, '|'));
								op_attr = &a;
							}
						}
					}
				}
				ImGui::PopID();
			}));

			if (open_select_standard_model)
			{
				ImGui::OpenPopup("select_standard_model");
				open_select_standard_model = false;
			}
			if (open_select_hash)
			{
				ImGui::OpenPopup("select_hash");
				open_select_hash = false;
			}
			if (ImGui::BeginPopup("select_standard_model"))
			{
				static const char* names[] = {
					"standard_plane",
					"standard_cube",
					"standard_sphere",
					"standard_cylinder",
					"standard_tri_prism"
				};
				for (auto n : names)
				{
					if (ImGui::Selectable(n))
					{
						std::filesystem::path v(n);
						auto changed = false;
						for (auto c : cc.components)
						{
							if (!op_attr->compare_to_value(c, &v))
							{
								op_attr->set_value(c, &v);
								changed = true;
							}
						}
						if (changed)
							ret_changed |= 2;
					}
				}

				ImGui::EndPopup();
			}
			if (ImGui::BeginPopup("select_hash"))
			{
				for (auto& c : hash_candidates)
				{
					if (ImGui::Selectable(c.c_str()))
					{
						uint v = sh(c.c_str());
						auto changed = false;
						for (auto c : cc.components)
						{
							if (!op_attr->compare_to_value(c, &v))
							{
								op_attr->set_value(c, &v);
								changed = true;
							}
						}
						if (changed)
							ret_changed |= 2;
					}
				}
				ImGui::EndPopup();
			}
			if (ret_changed_name != 0)
			{
				auto& str = ui.find_attribute(ret_changed_name)->name;
				for (auto e : entities)
				{
					if (auto ins = get_root_prefab_instance(e); ins)
						ins->mark_modification(e->file_id.to_string() + '|' + ui.name + '|' + str);
				}

				if ((ui.name_hash == "flame::cNavAgent"_h || ui.name_hash == "flame::cNavObstacle"_h) &&
					(ret_changed_name == "radius"_h || ret_changed_name == "height"_h))
				{
					for (auto& v : scene_window.views)
					{
						auto sv = (SceneView*)v.get();
						sv->show_navigation_frames = 3;
					}
				}
			}

			if (ui.name_hash == "flame::cNode"_h)
			{
				if (cc.components.size() == 1)
				{
					auto node = (cNodePtr)cc.components[0];
					ImGui::InputFloat4("qut", (float*)&node->qut, "%.3f", ImGuiInputTextFlags_ReadOnly);
					auto g_pos = node->global_pos();
					auto g_qut = node->global_qut();
					auto g_scl = node->global_scl();
					ImGui::InputFloat3("global pos", (float*)&g_pos, "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat4("global qut", (float*)&g_qut, "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("global scl", (float*)&g_scl, "%.3f", ImGuiInputTextFlags_ReadOnly);
				}
			}
			else if (ui.name_hash == "flame::cArmature"_h)
			{
				if (cc.components.size() == 1)
				{
					auto armature = (cArmaturePtr)cc.components[0];
					if (ImGui::Button("Reset"))
						armature->reset();
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
		}

		ImGui::PopID();
		editing_objects.pop();
		if (exit_editing)
			break;
	}

	ImGui::Dummy(vec2(0.f, 10.f));
	const float ButtonWidth = 100.f;
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ButtonWidth) * 0.5f);
	ImGui::SetNextItemWidth(ButtonWidth);
	static std::vector<UdtInfo*> comp_udts;
	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("add_component");
		comp_udts.clear();

		std::vector<UdtInfo*> temp1;
		std::vector<UdtInfo*> temp2;
		for (auto& ui : tidb.udts)
		{
			if (ui.second.base_class_name == "flame::Component")
			{
				if (ui.second.name.starts_with("flame::"))
					temp1.push_back(&ui.second);
				else
					temp2.push_back(&ui.second);
			}
		}
		std::sort(temp1.begin(), temp1.end(), [](const auto& a, const auto& b) {
			return a->name < b->name;
		});
		std::sort(temp2.begin(), temp2.end(), [](const auto& a, const auto& b) {
			return a->name < b->name;
		});
		comp_udts.insert(comp_udts.end(), temp1.begin(), temp1.end());
		comp_udts.insert(comp_udts.end(), temp2.begin(), temp2.end());
	}
	if (ImGui::BeginPopup("add_component"))
	{
		for (auto ui : comp_udts)
		{
			if (ImGui::Selectable(ui->name.c_str()))
			{
				auto changed = false;
				for (auto e : entities)
				{
					if (e->add_component_h(ui->name_hash))
					{
						if (auto ins = get_root_prefab_instance(e); ins)
						{
							auto idx = ins->find_modification(e->file_id.to_string() + '|' + ui->name + "|remove");
							if (idx == -1)
								ins->mark_modification(e->file_id.to_string() + '|' + ui->name + "|add");
							else
								ins->modifications.erase(ins->modifications.begin() + idx);
						}
						changed = true;
					}
				}
				if (changed)
				{
					auto es = entities;
					refresh(es);
					ret_changed |= 2;
				}
			}
		}
		ImGui::EndPopup();
	}

	return { ret_changed, ret_changed_name };
}

InspectorView::InspectorView() :
	InspectorView("Inspector##" + str(rand()))
{
}

InspectorView::InspectorView(const std::string& name) :
	View(&inspector_window, name)
{
	inspected_entities.inspector = this;
}

InspectorView::~InspectorView()
{
	inspected_entities.refresh({});
}

void InspectorView::on_draw()
{
	auto inspected_changed = last_select_frame < selection_changed_frame && !locked;
	if (inspected_changed)
	{
		last_select_frame = selection_changed_frame;

		inspected_type = selection.type;

		inspected_paths = selection.get_paths();
		if (inspected_obj_deletor && inspected_obj)
			inspected_obj_deletor(inspected_obj, inspected_obj_info);
		inspected_obj_deletor = nullptr;
		inspected_obj = nullptr;

		inspected_entities.entities = selection.get_entities();

		dirty = true;
	}

	if (dirty)
	{
		staging_vectors.clear();
		auto es = inspected_entities.entities;
		inspected_entities.refresh(es);
		dirty = false;
	}

	bool opened = true;
	ImGui::Begin(name.c_str(), &opened); 

	title_context_menu();

	if (inspected_type != Selection::tNothing)
	{
		if (ImGui::ToolButton(graphics::FontAtlas::icon_s(locked ? "lock"_h : "unlock"_h).c_str()))
		{
			locked = !locked;
			if (!locked)
				last_select_frame = 0;
		}
		ImGui::SameLine();
	}

	switch (inspected_type)
	{
	case Selection::tEntity:
		if (inspected_entities.entities.size() == 1)
		{
			static bool id_switch = true;
			if (id_switch)
				ImGui::Text("Instance ID: %s", inspected_entities.entities.front()->instance_id.to_string().c_str());
			else
				ImGui::Text("File ID: %s", inspected_entities.entities.front()->file_id.to_string().c_str());
			if (ImGui::IsItemClicked())
				id_switch = !id_switch;
		}
		else
			ImGui::Text("%d entities", (int)inspected_entities.entities.size());
		if (auto changed = inspected_entities.manipulate().first; changed >= 2)
		{
			if (!app.e_playing)
				app.prefab_unsaved = true;
			if (changed >= 3)
				dirty = true;
		}
		break;
	case Selection::tPath:
	{
		if (inspected_paths.size() == 1)
		{
			auto path = inspected_paths.front();
			ImGui::TextUnformatted(Path::reverse(path).string().c_str());
			ImGui::SameLine();
			if (ImGui::Button(graphics::FontAtlas::icon_s("location-crosshairs"_h).c_str()))
				project_window.ping(Path::get(path));
			auto ext = path.extension().wstring();
			SUW::to_lower(ext);
			if (ext == L".obj" || ext == L".fbx" || ext == L".gltf" || ext == L".glb")
			{
				static vec3 rotation = vec3(0, 0, 0);
				static float scaling = 1.f;
				static bool only_animation = false;
				ImGui::DragFloat3("Rotation", (float*)&rotation);
				ImGui::DragFloat("Scaling", &scaling);
				ImGui::Checkbox("Only Animation", &only_animation);
				if (ImGui::Button("Import"))
					graphics::import_scene(path, L"", rotation, scaling, only_animation);
			}
			else if (is_image_file(ext))
			{
				struct ImageInfo
				{
					uint chs;
					uint bpp;
					bool srgb;
				};
				auto& info = *(ImageInfo*)inspected_obj_info;

				if (inspected_changed)
				{
					auto image = graphics::Image::get(path);
					if (image)
					{
						inspected_obj = image;
						inspected_obj_deletor = [](void* obj, void* info) {
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

				if (inspected_obj)
				{
					auto image = (graphics::ImagePtr)inspected_obj;
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
						ImGui::Image(inspected_obj, (vec2)image->extent);
					if (view_type != 0)
						ImGui::PopImageViewType();
				}
			}
			else if (ext == L".fmat")
			{
				if (inspected_changed)
				{
					auto material = graphics::Material::get(path);
					if (material)
					{
						inspected_obj = material;
						inspected_obj_deletor = [](void* obj, void* info) {
							graphics::Material::release((graphics::MaterialPtr)obj);
						};
					}
				}

				if (inspected_obj)
				{
					auto material = (graphics::MaterialPtr)inspected_obj;
					editing_objects.emplace(EditingObjects(0, th<graphics::Material>(), &path, 1));
					auto changed = manipulate_udt(*TypeInfo::get<graphics::Material>()->retrive_ui(), (voidptr*)&material, 1).first;
					editing_objects.pop();
					if (changed >= 2)
					{
						material->save(path);
						auto asset = AssetManagemant::find(path);
						if (asset)
							asset->lwt = std::filesystem::last_write_time(path);
					}
				}
			}
			else if (ext == L".fmod")
			{
				if (inspected_changed)
				{
					auto model = graphics::Model::get(path);
					if (model)
					{
						inspected_obj = model;
						inspected_obj_deletor = [](void* obj, void* info) {
							graphics::Model::release((graphics::ModelPtr)obj);
						};
					}
				}

				if (inspected_obj)
				{
					auto model = (graphics::ModelPtr)inspected_obj;
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
					if (ImGui::Button("To Text"))
						model->save(path, false);
					if (ImGui::Button("To Binary"))
						model->save(path, true);
				}
			}
			else if (ext == L".fani")
			{
				if (inspected_changed)
				{
					auto animation = graphics::Animation::get(path);
					if (animation)
					{
						inspected_obj = animation;
						inspected_obj_deletor = [](void* obj, void* info) {
							graphics::Animation::release((graphics::AnimationPtr)obj);
						};
					}
				}

				if (inspected_obj)
				{
					auto animation = (graphics::AnimationPtr)inspected_obj;
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
			else if (ext == L".prefab")
			{
				if (inspected_changed)
				{
					inspected_obj = Entity::create();
					inspected_obj_deletor = [](void* obj, void* info) {
						delete (EntityPtr)obj;
					};

					((EntityPtr)inspected_obj)->load(path, true);
				}

				if (inspected_obj)
				{
					auto entity = (EntityPtr)inspected_obj;
					inspected_entities.refresh({ entity });
					inspected_entities.prefab_path = path;
					if (inspected_entities.manipulate().first >= 2)
					{
						entity->save(path, true);
						app.last_status = std::format("prefab saved: {}", path.string());
					}
				}
			}
			else if (ext == L".preset")
			{
				struct PresetInfo
				{
					UdtInfo* ui;
				};
				auto& info = *(PresetInfo*)inspected_obj_info;

				if (inspected_changed)
				{
					inspected_obj = load_preset_file(path, nullptr, &info.ui);
					inspected_obj_deletor = [](void* obj, void* _info) {
						auto& info = *(PresetInfo*)_info;
						info.ui->destroy_object(obj);
					};
				}

				if (inspected_obj)
				{
					editing_objects.emplace(EditingObjects(0, info.ui->name_hash, &path, 1));
					auto changed = manipulate_udt(*info.ui, (voidptr*)&inspected_obj, 1).first;
					editing_objects.pop();

					if (changed)
					{
						save_preset_file(path, inspected_obj, info.ui);
						auto asset = AssetManagemant::find(path);
						if (asset)
							asset->lwt = std::filesystem::last_write_time(path);
					}
				}
			}
			else if (ext == L".pipeline")
			{
				static UdtInfo* ser_ui = TypeInfo::get<graphics::PipelineInfo>()->retrive_ui()->transform_to_serializable();
				static std::vector<std::pair<std::string, std::string>> default_defines;

				if (inspected_changed)
				{
					inspected_obj = ser_ui->create_object();
					inspected_obj_deletor = [](void* obj, void* info) {
						ser_ui->destroy_object(obj);
						default_defines.clear();
					};

					std::ifstream file(path);
					LineReader res(file);
					res.read_block("");
					UnserializeTextSpec spec;
					spec.out_default_defines = &default_defines;
					unserialize_text(*ser_ui, res, 0, inspected_obj, spec);
					file.close();
				}

				if (inspected_obj)
				{
					uint changed = manipulate_variable(TypeInfo::get<decltype(default_defines)>(), "default defines", 0, 0, nullptr, nullptr, "", (voidptr*)&default_defines, 1, nullptr);
					changed |= manipulate_udt(*ser_ui, (voidptr*)&inspected_obj, 1).first;
					if (changed >= 2)
					{
						std::ofstream file(path);
						for (auto& d : default_defines)
							file << '%' << d.first << '=' << d.second << std::endl;
						SerializeTextSpec spec;
						spec.force_print_bar = true;
						serialize_text(*ser_ui, inspected_obj, file, "", spec);
						file.close();
					}
					if (ImGui::Button("Test Compile"))
					{
						auto pl = graphics::GraphicsPipeline::get(path, {});
						if (pl)
							graphics::GraphicsPipeline::release(pl);
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

							auto shaders = ser_ui->var_addr<std::vector<std::string>>(inspected_obj, "shaders"_h);

						}
					}
				}
			}
		}
		else
			ImGui::TextUnformatted("Multiple files selected.");
	}
		break;
	}

	ImGui::End();
	if (!opened)
		delete this;
}

InspectorWindow::InspectorWindow() :
	Window("Inspector")
{
}

void InspectorWindow::init()
{
	selection.callbacks.add([](uint caller) {
		if (caller != "inspector"_h)
			selection_changed_frame = frames;
	}, "inspector"_h);
}

void InspectorWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		new InspectorView;
}

void InspectorWindow::open_view(const std::string& name)
{
	new InspectorView(name);
}
