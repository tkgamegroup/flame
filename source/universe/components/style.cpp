#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>

namespace flame
{
	Vec4c get_color_2(EventReceiverState state, const std::vector<Vec4c>& colors)
	{
		if ((state & EventReceiverHovering) || (state & EventReceiverActive))
			return colors[1];
		return colors[0];
	}

	Vec4c get_color_3(EventReceiverState state, const std::vector<Vec4c>& colors)
	{
		auto lv = 0;
		if (state & EventReceiverHovering)
			lv++;
		if (state & EventReceiverActive)
			lv++;
		return colors[lv];
	}

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
				}, Mail::from_p(this));
				style();
			}
		}
	};

	void cStyleColor::style()
	{
		element->set_color(get_color_3(event_receiver->state, { color_normal, color_hovering, color_active }));
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
				}, Mail::from_p(this));
				style();
			}
		}
	};

	void cStyleColor2::style()
	{
		element->set_color(get_color_3(event_receiver->state, { color_normal[level], color_hovering[level], color_active[level] }));
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
				}, Mail::from_p(this));
				style();
			}
		}
	};

	void cStyleTextColor::style()
	{
		text->set_color(get_color_2(event_receiver->state, { color_normal, color_else }));
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
				}, Mail::from_p(this));
				style();
			}
		}
	};

	void cStyleTextColor2::style()
	{
		text->set_color(get_color_2(event_receiver->state, { color_normal[level], color_else[level] }));
	}

	cStyleTextColor2* cStyleTextColor2::create()
	{
		return new cStyleTextColor2Private;
	}
}
