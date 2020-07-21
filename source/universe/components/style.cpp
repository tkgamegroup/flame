//#include <flame/universe/components/element.h>
//#include <flame/universe/components/text.h>
//#include <flame/universe/components/event_receiver.h>
#include <flame/foundation/typeinfo.h>
#include "../entity_private.h"
#include "style_private.h"

namespace flame
{
	void cStylePrivate::add_rule(StateFlags state, const std::string& rule)
	{
		auto sp1 = SUS::split(rule, '=');
		if (sp1.size() != 2)
			return;

		auto sp2 = SUS::split(sp1[0], '.');
		if (sp2.size() < 2)
			return;

		auto e = (EntityPrivate*)entity;
		for (auto i = 0; i < sp2.size() - 2; i++)
		{
			e = e->find_child(sp2[i]);
			if (!e)
				return;
		}
		auto component = e->get_component(std::hash<std::string>()(sp2[sp2.size() - 2]));
		if (!component)
			return;

		auto udt = find_udt((std::string("flame::") + component->type_name).c_str());
		if (!udt)
			return;

		auto setter = udt->find_function(("set_" + sp2[sp2.size() - 1]).c_str());
		if (!setter || setter->get_type() != TypeInfo::get(TypeData, "void") || setter->get_parameters_count() != 1)
			return;

		auto type = setter->get_parameter(0);

		auto r = new Rule;
		r->state = state;
		r->target = component;
		r->type = type;
		r->setter = setter;
		r->data = type->create();
		type->unserialize(r->data, sp1[1].c_str());
		rules.emplace_back(r);

		on_entity_state_changed();
	}

	void cStylePrivate::on_entity_state_changed()
	{
		auto s = ((EntityPrivate*)entity)->state;
		for (auto& r : rules)
		{
			if (r->state == s)
			{
				void* parms[] = { r->type->get_tag() == TypePointer ? *(void**)r->data : r->data };
				r->setter->call(r->target, nullptr, parms);
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
