#include <flame/foundation/typeinfo.h>
#include "../entity_private.h"
#include "style_private.h"

namespace flame
{
	void cStylePrivate::set_rule(const std::string& _rule)
	{
		rule = _rule;
		SUS::remove_spaces(rule);
		targets.clear();

		auto et = TypeInfo::get(TypeEnumMulti, "flame::StateFlags");
		assert(et);

		auto sp1 = SUS::split(rule, '#');
		for (auto& i : sp1)
		{
			auto sp2 = SUS::split(SUS::trim(i), '?');
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
			auto c_type = sp4[sp4.size() - 2];
			auto c = e->get_component(std::hash<std::string>()(c_type));
			if (!c)
				c = e->get_component(std::hash<std::string>()("flame::" + c_type));
			if (!c)
				continue;

			auto udt = find_udt(c->type_name);
			if (!udt)
				continue;

			auto v_name = sp4[sp4.size() - 1];
			auto setter = udt->find_function(("set_" + v_name).c_str());
			if (!setter || setter->get_type() != TypeInfo::get(TypeData, "void") || setter->get_parameters_count() != 1)
				continue;

			Target* ptarget = nullptr;
			for (auto& t : targets)
			{
				if (t.c == c)
				{
					ptarget = &t;
					break;
				}
			}
			if (!ptarget)
			{
				targets.push_back({});
				ptarget = &targets.back();
			}
			ptarget->c = c;

			auto state = et->create();
			et->unserialize(state, sp2[0].c_str());
			Situation* psituation = nullptr;
			for (auto& s : ptarget->situations)
			{
				if (s.s == *(int*)state)
				{
					psituation = &s;
					break;
				}
			}
			if (!psituation)
			{
				ptarget->situations.push_back({});
				psituation = &ptarget->situations.back();
			}
			psituation->s = *(StateFlags*)state;

			auto type = setter->get_parameter(0);

			auto cmd = new Command;
			cmd->type = type;
			cmd->setter = setter;
			cmd->data = type->create();
			type->unserialize(cmd->data, sp3[1].c_str());
			psituation->cmds.emplace_back(cmd);

			et->destroy(state);
		}

		on_state_changed();

		Entity::report_data_changed(this, S<ch("rule")>::v);
	}

	void cStylePrivate::on_state_changed()
	{
		auto state = ((EntityPrivate*)entity)->state;
		for (auto& t : targets)
		{
			for (auto& s : t.situations)
			{
				if ((s.s & state) == s.s)
				{
					for (auto& c : s.cmds)
					{
						void* parms[] = { c->type->get_tag() == TypePointer ? *(void**)c->data : c->data };
						c->setter->call(t.c, nullptr, parms);
					}
					break;
				}
			}
		}
	}

	void cStylePrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageStateChanged:
			on_state_changed();
			break;
		}
	}

	cStyle* cStyle::create() 
	{
		return f_new<cStylePrivate>();
	}
}
