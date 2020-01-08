#include "../universe_private.h"
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/toggle.h>

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

			untoggled_color_normal = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 0.40f * 255.f);
			untoggled_color_hovering = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 1.00f * 255.f);
			untoggled_color_active = Vec4c(color(Vec3f(49.f, 0.43f, 0.97f)), 1.00f * 255.f);
			toggled_color_normal = default_style.button_color_normal;
			toggled_color_hovering = default_style.button_color_hovering;
			toggled_color_active = default_style.button_color_active;

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
				if (!toggled)
				{
					style->color_normal = untoggled_color_normal;
					style->color_hovering = untoggled_color_hovering;
					style->color_active = untoggled_color_active;
				}
				else
				{
					style->color_normal = toggled_color_normal;
					style->color_hovering = toggled_color_hovering;
					style->color_active = toggled_color_active;
				}
				style->style();
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto thiz = *(cTogglePrivate**)c;
						thiz->set_toggled(!thiz->toggled);
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor"))
			{
				style = (cStyleColor*)c;
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
