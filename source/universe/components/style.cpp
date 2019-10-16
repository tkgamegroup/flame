#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>

namespace flame
{
	struct cStyleColorPrivate : cStyleColor
	{
		void* state_changed_listener;

		cStyleColorPrivate(const Vec4c& _color_normal, const Vec4c& _color_hovering, const Vec4c& _color_active)
		{
			element = nullptr;
			event_receiver = nullptr;

			color_normal = _color_normal;
			color_hovering = _color_hovering;
			color_active = _color_active;

			state_changed_listener = nullptr;
		}

		~cStyleColorPrivate()
		{
			if (!entity->dying)
				event_receiver->state_changed_listeners.remove(state_changed_listener);
		}

		void style(EventReceiverState prev_state, EventReceiverState curr_state)
		{
			switch (curr_state)
			{
			case EventReceiverNormal:
				element->color = color_normal;
				break;
			case EventReceiverHovering:
				element->color = color_hovering;
				break;
			case EventReceiverActive:
				element->color = color_active;
				break;
			}
		}

		Component* copy()
		{
			return new cStyleColorPrivate(color_normal, color_hovering, color_active);
		}
	};

	void cStyleColor::style()
	{
		auto state = event_receiver->state;
		((cStyleColorPrivate*)this)->style(state, state);
	}

	void cStyleColor::on_enter_hierarchy(Component* c)
	{
		if (c)
		{
			const auto add_listener = [](cStyleColorPrivate* thiz) {
				thiz->state_changed_listener = thiz->event_receiver->state_changed_listeners.add([](void* c, EventReceiverState prev_state, EventReceiverState curr_state) {
					(*(cStyleColorPrivate**)c)->style(prev_state, curr_state);
				}, new_mail_p(thiz));
			};
			if (c == this)
			{
				element = (cElement*)(entity->find_component(cH("Element")));
				event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
				if (event_receiver)
					add_listener((cStyleColorPrivate*)this);
				style();
			}
			else if (c->type_hash == cH("Element"))
			{
				element = (cElement*)c;
				style();
			}
			else if (c->type_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
				add_listener((cStyleColorPrivate*)this);
				style();
			}
		}
	}

	Component* cStyleColor::copy()
	{
		return ((cStyleColorPrivate*)this)->copy();
	}

	cStyleColor* cStyleColor::create(const Vec4c& color_normal, const Vec4c& color_hovering, const Vec4c& color_active)
	{
		return new cStyleColorPrivate(color_normal, color_hovering, color_active);
	}

	struct cStyleTextColorPrivate : cStyleTextColor
	{
		void* state_changed_listener;

		cStyleTextColorPrivate(const Vec4c& _color_normal, const Vec4c& _color_else)
		{
			text = nullptr;
			event_receiver = nullptr;

			color_normal = _color_normal;
			color_else = _color_else;

			state_changed_listener = nullptr;
		}

		~cStyleTextColorPrivate()
		{
			if (!entity->dying)
				event_receiver->state_changed_listeners.remove(state_changed_listener);
		}

		void style(EventReceiverState prev_state, EventReceiverState curr_state)
		{
			switch (curr_state)
			{
			case EventReceiverNormal:
				text->color = color_normal;
				break;
			case EventReceiverHovering: case EventReceiverActive:
				text->color = color_else;
				break;
			}
		}

		Component* copy()
		{
			return new cStyleTextColorPrivate(color_normal, color_else);
		}
	};

	void cStyleTextColor::style()
	{
		auto state = event_receiver->state;
		((cStyleTextColorPrivate*)this)->style(state, state);
	}

	void cStyleTextColor::on_enter_hierarchy(Component* c)
	{
		if (c)
		{
			const auto add_listener = [](cStyleTextColorPrivate* thiz) {
				thiz->state_changed_listener = thiz->event_receiver->state_changed_listeners.add([](void* c, EventReceiverState prev_state, EventReceiverState curr_state) {
					(*(cStyleTextColorPrivate**)c)->style(prev_state, curr_state);
				}, new_mail_p(thiz));
			};
			if (c == this)
			{
				text = (cText*)(entity->find_component(cH("Text")));
				event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
				if (event_receiver)
					add_listener((cStyleTextColorPrivate*)this);
				style();
			}
			else if (c->type_hash == cH("Text"))
			{
				text = (cText*)c;
				style();
			}
			else if (c->type_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
				add_listener((cStyleTextColorPrivate*)this);
				style();
			}
		}
	}

	Component* cStyleTextColor::copy()
	{
		return ((cStyleTextColorPrivate*)this)->copy();
	}

	cStyleTextColor* cStyleTextColor::create(const Vec4c& color_normal, const Vec4c& color_else)
	{
		return new cStyleTextColorPrivate(color_normal, color_else);
	}
}
