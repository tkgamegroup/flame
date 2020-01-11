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
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto thiz = *(cTogglePrivate**)c;
						thiz->set_toggled(!thiz->toggled);
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				style = (cStyleColor2*)c;
				style->level = toggled ? 1 : 0;
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

	Entity* create_standard_toggle(graphics::FontAtlas* font_atlas, float font_size_scale, const wchar_t* text)
	{
		auto e_toggle = Entity::create();
		{
			auto c_element = cElement::create();
			auto r = ui::style(ui::FontSize).u()[0] * 0.5f;
			c_element->roundness_ = r;
			c_element->inner_padding_ = Vec4f(r, 2.f, r, 2.f);
			e_toggle->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->set_text(text);
			e_toggle->add_component(c_text);

			e_toggle->add_component(cEventReceiver::create());
			
			auto c_style = cStyleColor2::create();
			c_style->color_normal[0] = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 0.40f * 255.f);
			c_style->color_hovering[0] = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 1.00f * 255.f);
			c_style->color_active[0] = Vec4c(color(Vec3f(49.f, 0.43f, 0.97f)), 1.00f * 255.f);
			c_style->color_normal[1] = ui::style(ui::ButtonColorNormal).c();
			c_style->color_hovering[1] = ui::style(ui::ButtonColorHovering).c();
			c_style->color_active[1] = ui::style(ui::ButtonColorActive).c();
			e_toggle->add_component(c_style);

			e_toggle->add_component(cToggle::create());
		}
		return e_toggle;
	}
}
