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
			if (!entity->dying_)
				event_receiver->data_changed_listeners.remove(state_changed_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
				element = (cElement*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				state_changed_listener = event_receiver->data_changed_listeners.add([](void* c, Component*, uint hash, void*) {
					if (hash == FLAME_CHASH("state"))
						(*(cStyleColorPrivate**)c)->style();
				}, new_mail_p(this));
				style();
			}
		}

		Component* copy() override
		{
			return new cStyleColorPrivate(color_normal, color_hovering, color_active);
		}
	};

	void cStyleColor::style()
	{
		switch (event_receiver->state)
		{
		case EventReceiverNormal:
			element->set_color(color_normal);
			break;
		case EventReceiverHovering:
			element->set_color(color_hovering);
			break;
		case EventReceiverActive:
			element->set_color(color_active);
			break;
		}
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
			if (!entity->dying_)
				event_receiver->data_changed_listeners.remove(state_changed_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				state_changed_listener = event_receiver->data_changed_listeners.add([](void* c, Component*, uint hash, void*) {
					(*(cStyleTextColorPrivate**)c)->style();
				}, new_mail_p(this));
				style();
			}
		}

		Component* copy() override
		{
			return new cStyleTextColorPrivate(color_normal, color_else);
		}
	};

	void cStyleTextColor::style()
	{
		switch (event_receiver->state)
		{
		case EventReceiverNormal:
			text->color = color_normal;
			break;
		case EventReceiverHovering: case EventReceiverActive:
			text->color = color_else;
			break;
		}
	}

	cStyleTextColor* cStyleTextColor::create(const Vec4c& color_normal, const Vec4c& color_else)
	{
		return new cStyleTextColorPrivate(color_normal, color_else);
	}
}
