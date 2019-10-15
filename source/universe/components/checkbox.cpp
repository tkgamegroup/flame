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

		std::vector<std::unique_ptr<Closure<void(void* c, bool checked)>>> changed_listeners;

		cCheckboxPrivate()
		{
			element = nullptr;
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
		}

		~cCheckboxPrivate()
		{
			if (!entity->dying)
				event_receiver->mouse_listeners.remove(mouse_listener);
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
					auto thiz = *(cCheckboxPrivate**)c;
					thiz->set_checked(!thiz->checked);
				}

			}, new_mail_p(this));

			do_style();
		}
	};

	void* cCheckbox::add_changed_listener(void (*listener)(void* c, bool checked), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, bool checked)>;
		c->function = listener;
		c->capture = capture;
		((cCheckboxPrivate*)this)->changed_listeners.emplace_back(c);
		return c;
	}

	void cCheckbox::remove_changed_listener(void* ret_by_add)
	{
		auto& listeners = ((cCheckboxPrivate*)this)->changed_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cCheckbox::set_checked(bool _checked, bool trigger_changed)
	{
		auto thiz = (cCheckboxPrivate*)this;
		checked = _checked;
		thiz->do_style();
		if (trigger_changed)
		{
			for (auto& l : thiz->changed_listeners)
				l->function(l->capture.p, checked);
		}
	}

	void cCheckbox::start()
	{
		((cCheckboxPrivate*)this)->start();
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
