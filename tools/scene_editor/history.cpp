#include "selection.h"
#include "history.h"

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
}

void SelectHistory::redo()
{
	select(new_type, new_objects);
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
}

void AssetModifyHistory::redo()
{
	set_value(new_value);
}

void EntityModifyHistory::set_value(const std::vector<std::string>& values)
{
	if (app.e_prefab)
	{
		for (auto i = 0; i < ids.size(); i++)
		{
			if (auto e = app.e_prefab->find_with_instance_id(ids[i]))
			{
				UdtInfo* ui = nullptr;
				void* obj = nullptr;
				if (comp_type == 0)
				{
					ui = TypeInfo::get<Entity>()->retrive_ui();
					obj = e;
				}
				else
				{
					ui = find_udt(comp_type);
					obj = e->find_component(comp_type);
				}
				if (auto a = ui->find_attribute(attr_hash); a)
				{
					a->type->unserialize(values.size() == 1 ? values[0] : values[i], nullptr);
					a->set_value(obj, nullptr);
				}
			}
		}
		if (selection.type == Selection::tEntity)
			selection.callbacks.call("app"_h);
	}
}

void EntityModifyHistory::undo()
{
	set_value(old_values);
}

void EntityModifyHistory::redo()
{
	set_value(new_values);
}

void PrefabModifyHistory::undo()
{

}

void PrefabModifyHistory::redo()
{

}

int history_idx = -1;
std::vector<std::unique_ptr<History>> histories;
