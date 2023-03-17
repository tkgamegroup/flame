#include "selection.h"
#include "history.h"

#include <flame/foundation/typeinfo.h>

void AssetModifyHistory::set_value(const std::vector<std::string>& values)
{
	auto ui = find_udt(asset_type);

	auto ref_getter = ui->find_function("get"_h);
	auto ref_releaser = ui->find_function("release"_h);

	for (auto i = 0; i < paths.size(); i++)
	{
		auto obj = ref_getter->call<void*, const std::filesystem::path&>(nullptr, paths[i]);
		if (auto a = ui->find_attribute(attr_hash); a)
		{
			a->type->unserialize(values.size() == 1 ? values[0] : values[i], nullptr);
			a->set_value(obj, nullptr);
		}
		ref_releaser->call<void*, void*>(nullptr, obj);
	}
}

void AssetModifyHistory::undo()
{
	set_value(old_values);
}

void AssetModifyHistory::redo()
{
	set_value(new_values);
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

int history_idx = -1;
std::vector<std::unique_ptr<History>> histories;
