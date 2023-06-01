#include "../../foundation/typeinfo.h"
#include "../entity_private.h"
#include "data_updater_private.h"

namespace flame
{
	void cDataUpdaterPrivate::set_items(const std::vector<std::pair<std::string, std::string>>& _items)
	{
		if (items == _items)
			return;
		items = _items;

		modifiers.clear();
		modifiers.resize(items.size());
		for (auto i = 0; i < items.size(); ++i)
		{
			auto& dst = modifiers[i];
			
			auto& item = items[i];
			auto sp = SUS::split(item.first, '|');
			if (sp.size() != 2)
				continue;
			auto comp_hash = sh(sp.front().c_str());
			if (auto comp = entity->find_component_recursively(comp_hash); comp)
			{
				auto& ui = *find_udt(comp_hash);
				voidptr obj = comp;
				if (auto attr = ui.find_attribute(SUS::split(sp.back(), '.'), obj); attr && attr->type->tag == TagD)
				{
					auto expr = Expression::create(item.second);
					expr->set_variable("time", &time);
					if (expr->compile())
					{
						dst.attr = attr;
						dst.obj = obj;
						dst.expr.reset(expr);
					}
					else
						delete expr;
				}
			}
		}

		data_changed("items"_h);
	}

	void cDataUpdaterPrivate::start()
	{
		time = 0.f;

		for (auto& m : modifiers)
		{
			if (m.attr)
			{
				auto value = m.expr->get_value();
				m.attr->unserialize(m.obj, value);
			}
		}
	}

	void cDataUpdaterPrivate::update()
	{
		time += delta_time;

		for (auto& m : modifiers)
		{
			if (m.attr)
			{
				if (m.expr->update_bindings())
				{
					auto value = m.expr->get_value(false);
					m.attr->unserialize(m.obj, value);
				}
			}
		}
	}

	struct cDataUpdaterCreate : cDataUpdater::Create
	{
		cDataUpdaterPtr operator()(EntityPtr e) override
		{
			return new cDataUpdaterPrivate();
		}
	}cDataUpdater_create;
	cDataUpdater::Create& cDataUpdater::create = cDataUpdater_create;
}
