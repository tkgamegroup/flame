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
				event_receiver->remove_state_changed_listener(state_changed_listener);
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

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			state_changed_listener = event_receiver->add_state_changed_listener([](void* c, EventReceiverState prev_state, EventReceiverState curr_state) {
				(*(cStyleColorPrivate**)c)->style(prev_state, curr_state);
			}, new_mail_p(this));

			style(EventReceiverNormal, event_receiver->state);
		}

		Component* copy()
		{
			return new cStyleColorPrivate(color_normal, color_hovering, color_active);
		}
	};

	void cStyleColor::start()
	{
		((cStyleColorPrivate*)this)->start();
	}

	void cStyleColor::update()
	{
		((cStyleColorPrivate*)this)->update();
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
				event_receiver->remove_state_changed_listener(state_changed_listener);
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

		void start()
		{
			text = (cText*)(entity->find_component(cH("Text")));
			assert(text);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			state_changed_listener = event_receiver->add_state_changed_listener([](void* c, EventReceiverState prev_state, EventReceiverState curr_state) {
				(*(cStyleTextColorPrivate**)c)->style(prev_state, curr_state);
			}, new_mail_p(this));

			style(EventReceiverNormal, event_receiver->state);
		}

		Component* copy()
		{
			return new cStyleTextColorPrivate(color_normal, color_else);
		}
	};

	void cStyleTextColor::start()
	{
		((cStyleTextColorPrivate*)this)->start();
	}

	void cStyleTextColor::update()
	{
		((cStyleTextColorPrivate*)this)->update();
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
