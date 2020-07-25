//#include <flame/universe/components/element.h>
//#include <flame/universe/components/text.h>
//#include <flame/universe/components/event_receiver.h>
#include <flame/foundation/typeinfo.h>
#include "../entity_private.h"
#include "style_private.h"

namespace flame
{
	void cStylePrivate::set_rule(const std::string& rule)
	{
		cmds.clear();

		auto et = TypeInfo::get(TypeEnumMulti, "flame::StateFlags");
		assert(et);

		auto sp1 = SUS::split(rule, '#');
		for (auto& i : sp1)
		{
			auto sp2 = SUS::split(SUS::trim(i), ';');
			if (sp2.size() != 2)
				continue;
			auto sp3 = SUS::split(sp2[1], '=');
			if (sp3.size() != 2)
				continue;
			auto sp4 = SUS::split(sp3[0], '.');
			if (sp4.size() < 2)
				continue;

			auto e = (EntityPrivate*)entity;
			for (auto i = 0; i < sp4.size() - 2; i++)
			{
				e = e->find_child(sp4[i]);
				if (!e)
					break;
			}
			if (!e)
				continue;
			auto component = e->get_component(std::hash<std::string>()(sp4[sp4.size() - 2]));
			if (!component)
				continue;

			auto udt = find_udt((std::string("flame::") + component->type_name).c_str());
			if (!udt)
				continue;

			auto setter = udt->find_function(("set_" + sp4[sp4.size() - 1]).c_str());
			if (!setter || setter->get_type() != TypeInfo::get(TypeData, "void") || setter->get_parameters_count() != 1)
				continue;

			auto state = et->create();
			et->unserialize(state, sp2[0].c_str());
			auto it = std::find_if(cmds.begin(), cmds.end(), [&](const auto& a) {
				return a.first == *(int*)state;
			});
			if (it == cmds.end())
			{
				cmds.push_back({});
				it = cmds.end() - 1;
			}
			it->first = *(StateFlags*)state;

			auto type = setter->get_parameter(0);

			auto c = new Command;
			c->target = component;
			c->type = type;
			c->setter = setter;
			c->data = type->create();
			type->unserialize(c->data, sp3[1].c_str());
			it->second.emplace_back(c);

			et->destroy(state);
		}

		on_entity_state_changed();

		((EntityPrivate*)entity)->report_data_changed(this, S<ch("rule")>::v);
	}

	void cStylePrivate::on_entity_state_changed()
	{
		auto s = ((EntityPrivate*)entity)->state;
		for (auto& l : cmds)
		{
			if (l.first == s)
			{
				for (auto& c : l.second)
				{
					void* parms[] = { c->type->get_tag() == TypePointer ? *(void**)c->data : c->data };
					c->setter->call(c->target, nullptr, parms);
				}
				break;
			}
		}
	}

	cStyle* cStyle::create() 
	{
		return f_new<cStylePrivate>();
	}
}
