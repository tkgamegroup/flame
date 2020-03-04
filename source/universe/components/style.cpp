#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>

namespace flame
{
	struct cStyleColorPrivate : cStyleColor
	{
		void* state_changed_listener;

		cStyleColorPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			color_normal = Vec4c(0);
			color_hovering = Vec4c(0);
			color_active = Vec4c(0);

			state_changed_listener = nullptr;
		}

		~cStyleColorPrivate()
		{
			if (!entity->dying_)
				event_receiver->state_listeners.remove(state_changed_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
				element = (cElement*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				state_changed_listener = event_receiver->state_listeners.add([](void* c, EventReceiverState state) {
					(*(cStyleColorPrivate**)c)->style();
					return true;
				}, new_mail_p(this));
				style();
			}
		}

		Component* copy() override
		{
			auto copy = new cStyleColorPrivate;
			copy->color_normal = color_normal;
			copy->color_hovering = color_hovering;
			copy->color_active = color_active;
			return copy;
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

	cStyleColor* cStyleColor::create()
	{
		return new cStyleColorPrivate;
	}

	struct cStyleColor2Private : cStyleColor2
	{
		void* state_changed_listener;

		cStyleColor2Private()
		{
			element = nullptr;
			event_receiver = nullptr;

			level = 0;

			for (auto i = 0; i < 2; i++)
			{
				color_normal[i] = Vec4c(0);
				color_hovering[i] = Vec4c(0);
				color_active[i] = Vec4c(0);
			}

			state_changed_listener = nullptr;
		}

		~cStyleColor2Private()
		{
			if (!entity->dying_)
				event_receiver->state_listeners.remove(state_changed_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
				element = (cElement*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				state_changed_listener = event_receiver->state_listeners.add([](void* c, EventReceiverState state) {
					(*(cStyleColor2Private**)c)->style();
					return true;
				}, new_mail_p(this));
				style();
			}
		}

		Component* copy() override
		{
			auto copy = new cStyleColor2Private;
			copy->level = level;
			for (auto i = 0; i < 2; i++)
			{
				copy->color_normal[i] = color_normal[i];
				copy->color_hovering[i] = color_hovering[i];
				copy->color_active[i] = color_active[i];
			}
			return copy;
		}
	};

	void cStyleColor2::style()
	{
		switch (event_receiver->state)
		{
		case EventReceiverNormal:
			element->set_color(color_normal[level]);
			break;
		case EventReceiverHovering:
			element->set_color(color_hovering[level]);
			break;
		case EventReceiverActive:
			element->set_color(color_active[level]);
			break;
		}
	}

	cStyleColor2* cStyleColor2::create()
	{
		return new cStyleColor2Private;
	}

	struct cStyleTextColorPrivate : cStyleTextColor
	{
		void* state_changed_listener;

		cStyleTextColorPrivate()
		{
			text = nullptr;
			event_receiver = nullptr;

			color_normal = Vec4c(0);
			color_else = Vec4c(0);

			state_changed_listener = nullptr;
		}

		~cStyleTextColorPrivate()
		{
			if (!entity->dying_)
				event_receiver->state_listeners.remove(state_changed_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				state_changed_listener = event_receiver->state_listeners.add([](void* c, EventReceiverState state) {
					(*(cStyleTextColorPrivate**)c)->style();
					return true;
				}, new_mail_p(this));
				style();
			}
		}

		Component* copy() override
		{
			auto copy = new cStyleTextColorPrivate;
			copy->color_normal = color_normal;
			copy->color_else = color_else;
			return copy;
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

	cStyleTextColor* cStyleTextColor::create()
	{
		return new cStyleTextColorPrivate;
	}

	struct cStyleTextColor2Private : cStyleTextColor2
	{
		void* state_changed_listener;

		cStyleTextColor2Private()
		{
			text = nullptr;
			event_receiver = nullptr;

			level = 0;

			for (auto i = 0; i < 2; i++)
			{
				color_normal[i] = Vec4c(0);
				color_else[i] = Vec4c(0);
			}

			state_changed_listener = nullptr;
		}

		~cStyleTextColor2Private()
		{
			if (!entity->dying_)
				event_receiver->state_listeners.remove(state_changed_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				state_changed_listener = event_receiver->state_listeners.add([](void* c, EventReceiverState state) {
					(*(cStyleTextColor2Private**)c)->style();
					return true;
				}, new_mail_p(this));
				style();
			}
		}

		Component* copy() override
		{
			auto copy = new cStyleTextColor2Private;
			for (auto i = 0; i < 2; i++)
			{
				copy->color_normal[i] = color_normal[i];
				copy->color_else[i] = color_else[i];
			}
			return copy;
		}
	};

	void cStyleTextColor2::style()
	{
		switch (event_receiver->state)
		{
		case EventReceiverNormal:
			text->color = color_normal[level];
			break;
		case EventReceiverHovering: case EventReceiverActive:
			text->color = color_else[level];
			break;
		}
	}

	cStyleTextColor2* cStyleTextColor2::create()
	{
		return new cStyleTextColor2Private;
	}
}
