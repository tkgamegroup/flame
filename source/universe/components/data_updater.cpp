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

		expressions.clear();
		expressions.resize(items.size());
		for (auto i = 0; i < items.size(); ++i)
		{
			auto& dst = expressions[i];
			std::get<0>(dst) = nullptr;
			std::get<1>(dst) = nullptr;
			std::get<2>(dst).reset();
			
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
					auto expression = Expression::create(item.second);
					expression->set_variable("time", &time);
					if (expression->compile())
					{
						std::get<0>(dst) = attr;
						std::get<1>(dst) = obj;
						std::get<2>(dst).reset(expression);
					}
					else
						delete expression;
				}
			}
		}

		data_changed("items"_h);
	}

	void cDataUpdaterPrivate::start()
	{
		time = 0.f;
	}

	void cDataUpdaterPrivate::update()
	{
		time += delta_time;

		for (auto& e : expressions)
		{
			if (std::get<0>(e))
			{
				auto value = std::get<2>(e)->get_value();
				std::get<0>(e)->unserialize(std::get<1>(e), value);
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
