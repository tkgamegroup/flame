#include "../universe_private.h"
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/ui/style_stack.h>

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

			mouse_listener = nullptr;
		}

		~cCheckboxPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
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
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				style = (cStyleColor2*)c;
				style->level = checked ? 1 : 0;
				do_style();
			}
		}

		void do_style()
		{
			if (style)
			{
				style->level = checked ? 1 : 0;
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
			data_changed(FLAME_CHASH("checked"), nullptr);
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
			c_element->size_ = 16.f;
			c_element->frame_thickness_ = 3.f;
			c_element->frame_color_ = ui::style(ui::TextColorNormal).c();
			e_checkbox->add_component(c_element);

			e_checkbox->add_component(cEventReceiver::create());

			auto c_style = cStyleColor2::create();
			c_style->color_normal[0] = ui::style(ui::UncheckedColorNormal).c();
			c_style->color_hovering[0] = ui::style(ui::UncheckedColorHovering).c();
			c_style->color_active[0] = ui::style(ui::UncheckedColorActive).c();
			c_style->color_normal[1] = ui::style(ui::CheckedColorNormal).c();
			c_style->color_hovering[1] = ui::style(ui::CheckedColorHovering).c();
			c_style->color_active[1] = ui::style(ui::CheckedColorActive).c();
			e_checkbox->add_component(c_style);

			e_checkbox->add_component(cCheckbox::create());
		}

		return e_checkbox;
	}
}
