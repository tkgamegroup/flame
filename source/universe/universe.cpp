#include "../foundation/typeinfo.h"
#include "universe_private.h"
#include "entity_private.h"
#include "world_private.h"
// let changes of app.h trigger build (that app.h will copy to include dir)
#include "application.h"

namespace flame
{
	ModifierPrivate::ModifierPrivate(const Modifier& m, EntityPtr e, 
		const std::vector<std::pair<const char*, float*>>& extra_variables,
		const std::vector<std::pair<const char*, float>>& extra_consts)
	{
		auto sp = SUS::split(m.address, '|');
		if (sp.size() != 2)
			return;
		auto comp_hash = sh(sp.front().c_str());
		if (auto comp = e->find_component_recursively(comp_hash); comp)
		{
			auto& ui = *find_udt(comp_hash);
			obj = comp;
			if (attr = ui.find_attribute(SUS::split(sp.back(), '.'), obj); attr && attr->type->tag == TagD)
			{
				expr.reset(Expression::create(m.data));
				for (auto& v : extra_variables)
					expr->set_variable(v.first, v.second);
				for (auto& v : extra_consts)
					expr->set_const_value(v.first, v.second);
				if (!expr->compile())
				{
					attr = nullptr;
					obj = nullptr;
					expr.reset(nullptr);
				}
			}
		}
	}

	void ModifierPrivate::update(bool first_time)
	{
		if (attr)
		{
			if (expr)
			{
				auto changed = expr->update_bindings();
				if (!first_time && !changed)
					return;
				switch (attr->type->tag)
				{
				case TagD:
				{
					auto ti = (TypeInfo_Data*)attr->type;
					switch (ti->data_type)
					{
					case DataBool:
					{
						auto value = (bool)(int)expr->get_value();
						attr->set_value(obj, &value);
					}
						break;
					case DataInt:
					{
						auto value = (int)expr->get_value();
						attr->set_value(obj, &value);
					}
						break;
					case DataFloat:
					{
						auto value = expr->get_value();
						attr->set_value(obj, &value);
					}
						break;
					case DataString:
					case DataWString:
						attr->unserialize(obj, expr->get_string_value());
						break;
					}
				}
					break;
				}
			}
		}
	}

	void* universe_info()
	{
		return nullptr;
	}
}
