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
			auto sp2 = SUS::split(i, ';');
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

	cStylePrivate* cStylePrivate::create()
	{
		return f_new<cStylePrivate>();
	}

	cStyle* cStyle::create() { return cStylePrivate::create(); }

//	Vec4c get_color_2(EventReceiverState state, const std::vector<Vec4c>& colors)
//	{
//		if ((state & EventReceiverHovering) || (state & EventReceiverActive))
//			return colors[1];
//		return colors[0];
//	}
//
//	Vec4c get_color_3(EventReceiverState state, const std::vector<Vec4c>& colors)
//	{
//		auto lv = 0;
//		if (state & EventReceiverHovering)
//			lv++;
//		if (state & EventReceiverActive)
//			lv++;
//		return colors[lv];
//	}
//
//	struct cStyleColorPrivate : cStyleColor
//	{
//		void* state_changed_listener;
//
//		cStyleColorPrivate()
//		{
//			element = nullptr;
//			event_receiver = nullptr;
//
//			color_normal = Vec4c(0);
//			color_hovering = Vec4c(0);
//			color_active = Vec4c(0);
//
//			state_changed_listener = nullptr;
//		}
//
//		~cStyleColorPrivate()
//		{
//			if (!entity->dying_)
//				event_receiver->state_listeners.remove(state_changed_listener);
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityComponentAdded:
//				if (t == this)
//				{
//					element = entity->get_component(cElement);
//					event_receiver = entity->get_component(cEventReceiver);
//					assert(element);
//					assert(event_receiver);
//
//					state_changed_listener = event_receiver->state_listeners.add([](Capture& c, EventReceiverState state) {
//						c.thiz<cStyleColorPrivate>()->style();
//						return true;
//					}, Capture().set_thiz(this));
//					style();
//				}
//				break;
//			}
//		}
//	};
//
//	void cStyleColor::style()
//	{
//		element->set_color(get_color_3(event_receiver->state, { color_normal, color_hovering, color_active }));
//	}
//
//	cStyleColor* cStyleColor::create()
//	{
//		return new cStyleColorPrivate;
//	}
//
//	struct cStyleColor2Private : cStyleColor2
//	{
//		void* state_changed_listener;
//
//		cStyleColor2Private()
//		{
//			element = nullptr;
//			event_receiver = nullptr;
//
//			level = 0;
//
//			for (auto i = 0; i < 2; i++)
//			{
//				color_normal[i] = Vec4c(0);
//				color_hovering[i] = Vec4c(0);
//				color_active[i] = Vec4c(0);
//			}
//
//			state_changed_listener = nullptr;
//		}
//
//		~cStyleColor2Private()
//		{
//			if (!entity->dying_)
//				event_receiver->state_listeners.remove(state_changed_listener);
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityComponentAdded:
//				if (t == this)
//				{
//					element = entity->get_component(cElement);
//					event_receiver = entity->get_component(cEventReceiver);
//					assert(element);
//					assert(event_receiver);
//
//					state_changed_listener = event_receiver->state_listeners.add([](Capture& c, EventReceiverState state) {
//						c.thiz<cStyleColor2Private>()->style();
//						return true;
//					}, Capture().set_thiz(this));
//					style();
//				}
//				break;
//			}
//		}
//	};
//
//	void cStyleColor2::style()
//	{
//		element->set_color(get_color_3(event_receiver->state, { color_normal[level], color_hovering[level], color_active[level] }));
//	}
//
//	cStyleColor2* cStyleColor2::create()
//	{
//		return new cStyleColor2Private;
//	}
//
//	struct cStyleTextColorPrivate : cStyleTextColor
//	{
//		void* state_changed_listener;
//
//		cStyleTextColorPrivate()
//		{
//			text = nullptr;
//			event_receiver = nullptr;
//
//			color_normal = Vec4c(0);
//			color_else = Vec4c(0);
//
//			state_changed_listener = nullptr;
//		}
//
//		~cStyleTextColorPrivate()
//		{
//			if (!entity->dying_)
//				event_receiver->state_listeners.remove(state_changed_listener);
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityComponentAdded:
//				if (t == this)
//				{
//					text = entity->get_component(cText);
//					event_receiver = entity->get_component(cEventReceiver);
//					assert(text);
//					assert(event_receiver);
//
//					state_changed_listener = event_receiver->state_listeners.add([](Capture& c, EventReceiverState state) {
//						c.thiz<cStyleTextColorPrivate>()->style();
//						return true;
//					}, Capture().set_thiz(this));
//					style();
//				}
//				break;
//			}
//		}
//	};
//
//	void cStyleTextColor::style()
//	{
//		text->set_color(get_color_2(event_receiver->state, { color_normal, color_else }));
//	}
//
//	cStyleTextColor* cStyleTextColor::create()
//	{
//		return new cStyleTextColorPrivate;
//	}
//
//	struct cStyleTextColor2Private : cStyleTextColor2
//	{
//		void* state_changed_listener;
//
//		cStyleTextColor2Private()
//		{
//			text = nullptr;
//			event_receiver = nullptr;
//
//			level = 0;
//
//			for (auto i = 0; i < 2; i++)
//			{
//				color_normal[i] = Vec4c(0);
//				color_else[i] = Vec4c(0);
//			}
//
//			state_changed_listener = nullptr;
//		}
//
//		~cStyleTextColor2Private()
//		{
//			if (!entity->dying_)
//				event_receiver->state_listeners.remove(state_changed_listener);
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityComponentAdded:
//				if (t == this)
//				{
//					text = entity->get_component(cText);
//					event_receiver = entity->get_component(cEventReceiver);
//					assert(text);
//					assert(event_receiver);
//
//					state_changed_listener = event_receiver->state_listeners.add([](Capture& c, EventReceiverState state) {
//						c.thiz<cStyleTextColor2Private>()->style();
//						return true;
//					}, Capture().set_thiz(this));
//					style();
//				}
//				break;
//			}
//		}
//	};
//
//	void cStyleTextColor2::style()
//	{
//		text->set_color(get_color_2(event_receiver->state, { color_normal[level], color_else[level] }));
//	}
//
//	cStyleTextColor2* cStyleTextColor2::create()
//	{
//		return new cStyleTextColor2Private;
//	}
}
