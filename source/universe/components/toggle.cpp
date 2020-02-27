#include "../universe_private.h"
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/toggle.h>
#include <flame/universe/ui/style_stack.h>

namespace flame
{
	struct cTogglePrivate : cToggle
	{
		void* mouse_listener;

		cTogglePrivate()
		{
			event_receiver = nullptr;
			style = nullptr;

			toggled = false;

			mouse_listener = nullptr;
		}

		~cTogglePrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style()
		{
			if (style)
			{
				style->level = toggled ? 1 : 0;
				style->style();
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto thiz = *(cTogglePrivate**)c;
						thiz->set_toggled(!thiz->toggled);
					}
					return true;
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				style = (cStyleColor2*)c;
				do_style();
			}
		}
	};

	void cToggle::set_toggled(bool _toggled, bool trigger_changed)
	{
		auto thiz = (cTogglePrivate*)this;
		toggled = _toggled;
		thiz->do_style();
		if (trigger_changed)
			data_changed(FLAME_CHASH("toggled"), nullptr);
	}

	cToggle* cToggle::create()
	{
		return new cTogglePrivate();
	}
}
