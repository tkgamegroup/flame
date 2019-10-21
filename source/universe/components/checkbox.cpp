#include "../universe_private.h"
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct cCheckboxPrivate : cCheckbox
	{
		void* mouse_listener;

		cCheckboxPrivate()
		{
			event_receiver = nullptr;
			style = nullptr;

			checked = false;

			unchecked_color_normal = default_style.unchecked_color_normal;
			unchecked_color_hovering = default_style.unchecked_color_hovering;
			unchecked_color_active = default_style.unchecked_color_active;
			checked_color_normal = default_style.checked_color_normal;
			checked_color_hovering = default_style.checked_color_hovering;
			checked_color_active = default_style.checked_color_active;

			mouse_listener = nullptr;

			changed_listeners.hub = new ListenerHub;
		}

		~cCheckboxPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);

			delete (ListenerHub*)changed_listeners.hub;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto thiz = *(cCheckboxPrivate**)c;
						thiz->set_checked(!thiz->checked);
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == cH("StyleColor"))
			{
				style = (cStyleColor*)c;
				do_style();
			}
		}

		void do_style()
		{
			if (style)
			{
				if (!checked)
				{
					style->color_normal = unchecked_color_normal;
					style->color_hovering = unchecked_color_hovering;
					style->color_active = unchecked_color_active;
				}
				else
				{
					style->color_normal = checked_color_normal;
					style->color_hovering = checked_color_hovering;
					style->color_active = checked_color_active;
				}
				style->style();
			}
		}
	};

	void cCheckbox::set_checked(bool _checked, bool trigger_changed)
	{
		auto thiz = (cCheckboxPrivate*)this;
		checked = _checked;
		thiz->do_style();
		if (trigger_changed)
		{
			auto& listeners = ((ListenerHub*)changed_listeners.hub)->listeners;
			for (auto& l : listeners)
				((void(*)(void*, bool))l->function)(l->capture.p, checked);
		}
	}

	cCheckbox* cCheckbox::create()
	{
		return new cCheckboxPrivate();
	}

	Entity* create_standard_checkbox()
	{
		auto e_checkbox = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->size = 16.f;
			c_element->frame_thickness = 3.f;
			c_element->frame_color = default_style.text_color_normal;
			e_checkbox->add_component(c_element);

			e_checkbox->add_component(cEventReceiver::create());

			e_checkbox->add_component(cStyleColor::create());

			e_checkbox->add_component(cCheckbox::create());
		}

		return e_checkbox;
	}
}
