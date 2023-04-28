#include "selection.h"
#include "history.h"

#include <flame/foundation/typeinfo.h>

void SelectHistory::select(Selection::Type type, const std::vector<void*> objects)
{

}

void SelectHistory::undo()
{

}

void SelectHistory::redo()
{

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
