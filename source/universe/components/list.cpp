#include "../../foundation/typeinfo.h"
#include "../entity_private.h"
#include "list_private.h"

namespace flame
{
	void cListPrivate::set_prefab_name(const std::filesystem::path& name)
	{
		if (prefab_name == name)
			return;
		prefab_name = name;
		refresh_items();
		data_changed("prefab_name"_h);
	}

	void cListPrivate::set_count(uint _count)
	{
		if (count == _count)
			return;
		count = _count;
		refresh_items();
		data_changed("count"_h);
	}

	void cListPrivate::set_modifiers(const std::vector<std::tuple<std::string, std::string, std::string>>& _modifiers)
	{
		if (modifiers == _modifiers)
			return;
		modifiers = _modifiers;
		refresh_items();
		data_changed("modifiers"_h);
	}

	void cListPrivate::refresh_items()
	{
		for (auto e : items)
			e->remove_from_parent();
		items.clear();
		std::vector<std::tuple<uint, uint, std::string>> processed_modifiers; // component hash, attribute hash, expression
		for (auto& mod : modifiers)
		{
			auto comp_hash = sh(std::get<0>(mod).c_str());
			auto attr_hash = sh(std::get<1>(mod).c_str());
			processed_modifiers.emplace_back(comp_hash, attr_hash, std::get<2>(mod));
		}

		for (auto i = 0; i < count; i++)
		{
			auto e = Entity::create();
			e->name = std::format("item_{}", i);
			if (!prefab_name.empty())
			{
				e->load(prefab_name);
				for (auto& mod : processed_modifiers)
				{
					auto comp_hash = std::get<0>(mod);
					auto attr_hash = std::get<1>(mod);
					auto value = std::get<2>(mod);
					SUS::replace_all(value, "%i", str(i));

					if (auto comp = e->find_component_recursively(comp_hash); comp)
					{
						auto& ui = *find_udt(comp_hash);
						if (auto attr = ui.find_attribute(attr_hash); attr)
							attr->unserialize(comp, value);
					}
				}
			}
			entity->add_child(e);
		}
	}

	struct cListCreate : cList::Create
	{
		cListPtr operator()(EntityPtr e) override
		{
			return new cListPrivate();
		}
	}cList_create;
	cList::Create& cList::create = cList_create;
}
