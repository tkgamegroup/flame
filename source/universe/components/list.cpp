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

		for (auto i = 0; i < count; i++)
		{
			auto e = Entity::create();
			e->name = "item" + str(i);
			if (!prefab_name.empty())
			{
				e->load(prefab_name);
				for (auto& mod : modifiers)
				{
					auto comp_hash = sh(std::get<0>(mod).c_str());
					if (auto comp = e->find_component_recursively(comp_hash); comp)
					{
						auto& ui = *find_udt(comp_hash);
						voidptr obj = comp;
						if (auto attr = ui.find_attribute(SUS::split(std::get<1>(mod), '.'), obj); attr && attr->type->tag == TagD)
						{
							auto expression = Expression::create(std::get<2>(mod));
							expression->set_const_string("i", str(i));
							expression->compile();
							attr->unserialize(obj, expression->get_value());
						}
					}
				}
			}
			e->tag = e->tag | TagNotSerialized;
			entity->add_child(e);
			items.push_back(e);
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
