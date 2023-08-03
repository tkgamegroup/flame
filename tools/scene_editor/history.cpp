#include "selection.h"
#include "history.h"

static auto ui_entity = TypeInfo::get<Entity>()->retrive_ui();
static auto ui_component = TypeInfo::get<Component>()->retrive_ui();

SelectHistory::~SelectHistory()
{
	auto destroy = [](Selection::Type& type, std::vector<void*>& objects) {
		switch (type)
		{
		case Selection::tPath:
			for (auto obj : objects)
				delete (std::filesystem::path*)obj;
			break;
		case Selection::tEntity:
			for (auto obj : objects)
				delete (std::string*)obj;
			break;
		}
	};
	destroy(old_type, old_objects);
	destroy(new_type, new_objects);
}

void SelectHistory::init(int old_or_new)
{
	auto& type = old_or_new ? new_type : old_type;
	auto& objects = old_or_new ? new_objects : old_objects;
	type = selection.type;
	objects.resize(selection.objects.size());
	for (auto i = 0; i < objects.size(); i++)
	{
		switch (type)
		{
		case Selection::tPath:
		{
			auto p = new std::filesystem::path;
			*p = *(std::filesystem::path*)selection.objects[i];
			objects[i] = p;
		}
			break;
		case Selection::tEntity:
		{
			auto p = new std::string;
			*p = ((EntityPtr)selection.objects[i])->instance_id.to_string();
			objects[i] = p;
		}
			break;
		}
	}
}

void SelectHistory::select(Selection::Type type, const std::vector<void*> objects)
{
	switch (type)
	{
	case Selection::tNothing:
		selection.clear_ll();
		selection.callbacks.call("history"_h);
		break;
	case Selection::tPath:
	{
		std::vector<std::filesystem::path> paths(objects.size());
		for (auto i = 0; i < objects.size(); i++)
			paths[i] = *(std::filesystem::path*)objects[i];
		selection.select_ll(paths);
		selection.callbacks.call("history"_h);
	}
		break;
	case Selection::tEntity:
	{
		std::vector<EntityPtr> entities;
		if (app.e_prefab)
		{
			for (auto i = 0; i < objects.size(); i++)
			{
				GUID guid;
				guid.from_string(*(std::string*)objects[i]);
				if (auto e = app.e_prefab->find_with_instance_id(guid); e)
					entities.push_back(e);
			}
		}
		selection.select_ll(entities);
		selection.callbacks.call("history"_h);
	}
		break;
	}
}

void SelectHistory::undo()
{
	select(old_type, old_objects);
	app.last_status = "Undo select";
}

void SelectHistory::redo()
{
	select(new_type, new_objects);
	app.last_status = "Redo select";
}

void AssetModifyHistory::set_value(const std::string& value)
{
	auto ui = find_udt(asset_type);

	auto func_get = ui->find_function("get"_h);
	auto func_release = ui->find_function("release"_h);
	auto func_save = ui->find_function("save"_h);

	auto obj = func_get->call<void*, const std::filesystem::path&>(nullptr, path);
	if (auto a = ui->find_attribute(attr_hash); a)
	{
		a->type->unserialize(value, nullptr);
		a->set_value(obj, nullptr);
	}
	func_save->call<void, const std::filesystem::path&>(obj, path);
	func_release->call<void, void*>(nullptr, obj);
}

void AssetModifyHistory::undo()
{
	set_value(old_value);
	app.last_status = "Undo modify asset";
}

void AssetModifyHistory::redo()
{
	set_value(new_value);
	app.last_status = "Redo modify asset";
}

void EntityModifyHistory::set_values(const std::vector<std::string>& values)
{
	if (app.e_prefab)
	{
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto e = app.e_prefab->find_with_instance_id(ids[i]); e)
			{
				UdtInfo* ui = TypeInfo::get<Entity>()->retrive_ui();
				void* obj = e;
				if (comp_type)
				{
					ui = find_udt(comp_type);
					obj = e->find_component(comp_type);
				}
				if (ui && obj)
				{
					if (auto a = ui->find_attribute(attr_hash); a)
					{
						a->type->unserialize(values.size() == 1 ? values[0] : values[i], nullptr);
						a->set_value(obj, nullptr);
					}
				}
			}
		}

		selection.callbacks.call("app"_h);
	}
}

void EntityModifyHistory::undo()
{
	set_values(old_values);
	app.last_status = "Undo modify entity";
}

void EntityModifyHistory::redo()
{
	set_values(new_values);
	app.last_status = "Redo modify entity";
}

void EntityContent::init(EntityPtr e)
{
	for (auto& c : e->components)
		components.push_back(c->type_hash);

	for (auto& a : ui_entity->attributes)
	{
		if (auto value = a.serialize(e); value != a.default_value)
			values.emplace_back(0, a.name_hash, value);
	}
	for (auto& c : e->components)
	{
		for (auto& a : ui_component->attributes)
		{
			if (auto value = a.serialize(c.get()); value != a.default_value)
				values.emplace_back(c->type_hash, a.name_hash, value);
		}
		for (auto& a : find_udt(c->type_hash)->attributes)
		{
			if (auto value = a.serialize(c.get()); value != a.default_value)
				values.emplace_back(c->type_hash, a.name_hash, value);
		}
	}
}

void EntityHistory::recreate_entities(const std::vector<GUID>& parents, const std::vector<uint>& indices)
{
	if (app.e_prefab)
	{
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto p = app.e_prefab->find_with_instance_id(parents[i]); p)
			{
				auto e = Entity::create();
				auto& content = contents[i];
				for (auto h : content.components)
					e->add_component_h(h);
				for (auto& item : content.values)
				{
					auto ui = TypeInfo::get<Entity>()->retrive_ui();
					void* obj = e;
					if (std::get<0>(item))
					{
						ui = find_udt(std::get<0>(item));
						obj = e->find_component(std::get<0>(item));
					}
					if (ui && obj)
					{
						if (auto a = ui->find_attribute(std::get<1>(item)); a)
						{
							a->type->unserialize(std::get<2>(item), nullptr);
							a->set_value(obj, nullptr);
						}
					}
				}
				p->add_child(e, indices[i]);
			}
		}

		selection.callbacks.call("app"_h);
	}
}

void EntityHistory::remove_entities()
{
	if (app.e_prefab)
	{
		auto is_selecting_entities = selection.type == Selection::tEntity;
		auto selected_entities = selection.get_entities();
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto e = app.e_prefab->find_with_instance_id(ids[i]); e)
			{
				e->remove_from_parent();
				if (is_selecting_entities)
				{
					if (auto it = std::find(selected_entities.begin(), selected_entities.end(), e); it != selected_entities.end())
						selected_entities.erase(it);
				}
			}
		}
		if (is_selecting_entities)
			selection.select(selected_entities);
	}
}

void EntityHistory::undo()
{
	if (!old_parents.empty())
	{
		recreate_entities(old_parents, old_indices);
		app.last_status = "Undo remove entities";
	}
	else
	{
		remove_entities();
		app.last_status = "Undo create entities";
	}
}

void EntityHistory::redo()
{
	if (!old_parents.empty())
	{
		remove_entities();
		app.last_status = "Redo remove entities";
	}
	else
	{
		recreate_entities(new_parents, new_indices);
		app.last_status = "Redo create entities";
	}
}

void EntityPositionHistory::set_positions(const std::vector<GUID>& parents, const std::vector<uint>& indices)
{
	if (app.e_prefab)
	{
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto e = app.e_prefab->find_with_instance_id(ids[i]); e)
			{
				if (auto p = app.e_prefab->find_with_instance_id(parents[i]); p)
				{
					e->parent->remove_child(e, false);
					p->add_child(e, indices[i]);
				}
			}
		}
	}
}

void EntityPositionHistory::undo()
{
	set_positions(old_parents, old_indices);
	app.last_status = "Undo reposition entities";
}

void EntityPositionHistory::redo()
{
	set_positions(new_parents, new_indices);
	app.last_status = "Redo reposition entities";
}

void ComponentHistory::recreate_components(const std::vector<GUID>& ids, const std::vector<uint>& indices)
{
	if (app.e_prefab)
	{
		auto ui = find_udt(comp_type);
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto e = app.e_prefab->find_with_instance_id(ids[i]); e)
			{
				auto c = e->add_component_h(comp_type);
				if (!contents.empty())
				{
					for (auto& item : contents[i])
					{
						if (auto a = ui->find_attribute(item.first); a)
						{
							a->type->unserialize(item.second, nullptr);
							a->set_value(c, nullptr);
						}
					}
				}
				e->reposition_component(e->find_component_i(comp_type), indices[i]);
			}
		}

		selection.callbacks.call("app"_h);
	}
}

void ComponentHistory::remove_components(const std::vector<GUID>& ids)
{
	if (app.e_prefab)
	{
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto e = app.e_prefab->find_with_instance_id(ids[i]); e)
				e->remove_component_h(comp_type);
		}

		selection.callbacks.call("app"_h);
	}
}

void ComponentHistory::undo()
{
	if (!old_ids.empty())
	{
		recreate_components(old_ids, old_indices);
		app.last_status = "Undo remove components";
	}
	else
	{
		remove_components(new_ids);
		app.last_status = "Undo create components";
	}
}

void ComponentHistory::redo()
{
	if (!old_ids.empty())
	{
		remove_components(old_ids);
		app.last_status = "Redo remove components";
	}
	else
	{
		recreate_components(new_ids, new_indices);
		app.last_status = "Redo create components";
	}
}

void ComponentPositionHistory::set_positions(const std::vector<uint>& indices)
{
	if (app.e_prefab)
	{
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto e = app.e_prefab->find_with_instance_id(ids[i]); e)
			{
				if (auto idx = e->find_component_i(comp_type); idx != -1)
					e->reposition_component(idx, indices[i]);
			}
		}

		selection.callbacks.call("app"_h);
	}
}

void ComponentPositionHistory::undo()
{
	set_positions(old_indices);
	app.last_status = "Undo reposition components";
}

void ComponentPositionHistory::redo()
{
	set_positions(new_indices);
	app.last_status = "Redo reposition components";
}

void PrefabModifyHistory::set_value(const std::string& value)
{
	if (std::filesystem::exists(path))
	{
		auto e = Entity::create();
		e->load(path, true);

		UdtInfo* ui = TypeInfo::get<Entity>()->retrive_ui();
		void* obj = e;
		if (comp_type)
		{
			ui = find_udt(comp_type);
			obj = e->find_component(comp_type);
		}
		if (ui && obj)
		{
			if (auto a = ui->find_attribute(attr_hash); a)
			{
				a->type->unserialize(value, nullptr);
				a->set_value(obj, nullptr);
			}
		}

		e->save(path, true);
		delete e;
	}
}

void PrefabModifyHistory::undo()
{
	set_value(old_value);
	app.last_status = "Undo modify prefab";
}

void PrefabModifyHistory::redo()
{
	set_value(new_value);
	app.last_status = "Redo modify prefab";
}

int history_idx = -1;
std::vector<std::unique_ptr<History>> histories;
