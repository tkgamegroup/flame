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

		std::vector<std::unique_ptr<Closure<void(void* c, bool toggled)>>> changed_listeners;

		cTogglePrivate()
		{
			element = nullptr;
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
			if (!entity->dying)
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

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			style = (cStyleColor*)(entity->find_component(cH("StyleColor")));

			mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_clicked(action, key))
				{
					auto thiz = *(cTogglePrivate**)c;
					thiz->set_toggled(!thiz->toggled);
				}

			}, new_mail_p(this));

			do_style();
		}
	};

	void* cToggle::add_changed_listener(void (*listener)(void* c, bool checked), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, bool toggled)>;
		c->function = listener;
		c->capture = capture;
		((cTogglePrivate*)this)->changed_listeners.emplace_back(c);
		return c;
	}

	void cToggle::remove_changed_listener(void* ret_by_add)
	{
		auto& listeners = ((cTogglePrivate*)this)->changed_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cToggle::set_toggled(bool _toggled, bool trigger_changed)
	{
		auto thiz = (cTogglePrivate*)this;
		toggled = _toggled;
		thiz->do_style();
		if (trigger_changed)
		{
			for (auto& l : thiz->changed_listeners)
				l->function(l->capture.p, toggled);
		}
	}

	void cToggle::start()
	{
		((cTogglePrivate*)this)->start();
	}

	cToggle* cToggle::create()
	{
		return new cTogglePrivate();
	}
}
